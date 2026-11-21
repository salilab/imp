/**
 *  \file cn_docking.cpp
 *  \brief Generate candidate cyclic (Cn) assemblies of a monomer.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#include <IMP/cnmultifit/internal/cn_docking.h>
#include <IMP/multifit/internal/FFTWGrid.h>
#include <IMP/multifit/internal/FFTWPlan.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/exception.h>
#include <IMP/log.h>
#include <IMP/Pointer.h>
#include <algorithm>
#include <cmath>
#include <limits>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

namespace {

const double PI = 3.14159265358979323846;

typedef algebra::Vector3D V3;

//! Simple 3x3 matrix, faster than Rotation3D for many applications
struct Mat3 {
  double m[3][3];
  V3 operator*(const V3 &v) const {
    return V3(m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2],
              m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
              m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]);
  }
  Mat3 get_transpose() const {
    Mat3 r;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        r.m[i][j] = m[j][i];
      }
    }
    return r;
  }
};

//! Rotation about the unit vector u by angle (Rodrigues' formula)
Mat3 get_rotation_matrix(const V3 &u, double angle) {
  double s = std::sin(angle), c = 1. - std::cos(angle);
  double K[3][3] = {{0., -u[2], u[1]}, {u[2], 0., -u[0]}, {-u[1], u[0], 0.}};
  Mat3 r;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      double k2 = 0.;
      for (int k = 0; k < 3; ++k) k2 += K[i][k] * K[k][j];
      r.m[i][j] = (i == j ? 1. : 0.) + s * K[i][j] + c * k2;
    }
  }
  return r;
}

//! Get an orthonormal basis (e1, e2) of the plane perpendicular to u
void get_plane_basis(const V3 &u, V3 &e1, V3 &e2) {
  V3 a = std::abs(u[0]) < 0.9 ? V3(1., 0., 0.) : V3(0., 1., 0.);
  e1 = algebra::get_vector_product(u, a).get_unit_vector();
  e2 = algebra::get_vector_product(u, e1);
}

//! Approximately uniform unit vectors on the z>=0 hemisphere
/** Uses a Fibonacci lattice with the given approximate angular spacing. */
algebra::Vector3Ds get_hemisphere_directions(double step_deg) {
  double step = step_deg * PI / 180.;
  int n = std::max(8, static_cast<int>(std::round(2. * PI / (step * step))));
  algebra::Vector3Ds ret;
  ret.reserve(n);
  double golden = PI * (3. - std::sqrt(5.));
  for (int i = 0; i < n; ++i) {
    double fi = i + 0.5;
    double z = 1. - fi / n;
    double phi = fi * golden;
    double r = std::sqrt(1. - z * z);
    ret.push_back(V3(r * std::cos(phi), r * std::sin(phi), z));
  }
  return ret;
}

//! Smallest integer >= n with no prime factors other than 2, 3, 5
int get_fft_friendly_size(int n) {
  for (int m = std::max(n, 1);; ++m) {
    int k = m;
    for (int p : {2, 3, 5}) {
      while (k % p == 0) k /= p;
    }
    if (k == 1) return m;
  }
}

//! Piecewise-linear contact potential
/** A function of the distance (A) from a neighbor atom to the closest
    monomer atom: strongly negative for clashes, +1 for contacts, and 0
    beyond the cutoff. */
class ContactPotential {
  std::vector<std::pair<double, double> > knots_;

 public:
  ContactPotential(bool ca_only) {
    if (ca_only) {
      // One point per residue; CA-CA contacts are ~5-8A
      knots_ = {{0., -8.}, {2.5, -8.}, {3.5, -1.}, {4.0, 1.},
                {7.5, 1.}, {8.5, 0.}};
    } else {
      // Heavy atoms; contacts are ~3.3-5A (lenient, as side chains
      // would rearrange at the interface)
      knots_ = {{0., -8.}, {1.0, -8.}, {1.8, -3.}, {2.4, 0.},
                {2.6, 1.}, {4.6, 1.}, {5.3, 0.}};
    }
  }

  double get_cutoff() const { return knots_.back().first; }

  double operator()(double d) const {
    if (d <= knots_[0].first) return knots_[0].second;
    for (unsigned i = 1; i < knots_.size(); ++i) {
      if (d <= knots_[i].first) {
        double f = (d - knots_[i - 1].first) /
                   (knots_[i].first - knots_[i - 1].first);
        return knots_[i - 1].second + f * (knots_[i].second -
                                           knots_[i - 1].second);
      }
    }
    return 0.;
  }
};

//! Exact 1D squared distance transform (Felzenszwalb & Huttenlocher)
void distance_transform_1d(const std::vector<double> &f,
                           std::vector<double> &d, std::vector<int> &v,
                           std::vector<double> &z) {
  int n = f.size();
  int k = 0;
  v[0] = 0;
  z[0] = -std::numeric_limits<double>::max();
  z[1] = std::numeric_limits<double>::max();
  for (int q = 1; q < n; ++q) {
    double s;
    while (true) {
      s = ((f[q] + double(q) * q) - (f[v[k]] + double(v[k]) * v[k])) /
          (2. * q - 2. * v[k]);
      // z[0] is -infinity, so this always terminates with k >= 0
      if (s <= z[k]) {
        --k;
      } else {
        break;
      }
    }
    ++k;
    v[k] = q;
    z[k] = s;
    z[k + 1] = std::numeric_limits<double>::max();
  }
  k = 0;
  for (int q = 0; q < n; ++q) {
    while (z[k + 1] < q) ++k;
    double dq = q - v[k];
    d[q] = dq * dq + f[v[k]];
  }
}

//! Regular 3D grid of floats, stored with z varying fastest
struct Grid3 {
  int dim[3];
  std::vector<float> data;
  void resize(int nx, int ny, int nz, float val) {
    dim[0] = nx;
    dim[1] = ny;
    dim[2] = nz;
    data.assign(static_cast<size_t>(nx) * ny * nz, val);
  }
  size_t index(int i, int j, int k) const {
    return (static_cast<size_t>(i) * dim[1] + j) * dim[2] + k;
  }
  float &operator()(int i, int j, int k) { return data[index(i, j, k)]; }
  float operator()(int i, int j, int k) const {
    return data[index(i, j, k)];
  }
  //! Trilinear interpolation at grid coordinates g; zero outside the grid
  double interpolate(double gx, double gy, double gz) const {
    if (gx < 0. || gy < 0. || gz < 0. || gx > dim[0] - 1 ||
        gy > dim[1] - 1 || gz > dim[2] - 1) {
      return 0.;
    }
    int i = std::min(static_cast<int>(gx), dim[0] - 2);
    int j = std::min(static_cast<int>(gy), dim[1] - 2);
    int k = std::min(static_cast<int>(gz), dim[2] - 2);
    double fx = gx - i, fy = gy - j, fz = gz - k;
    const float *p = &data[index(i, j, k)];
    size_t sy = dim[2], sx = static_cast<size_t>(dim[1]) * dim[2];
    double c00 = p[0] * (1. - fz) + p[1] * fz;
    double c01 = p[sy] * (1. - fz) + p[sy + 1] * fz;
    double c10 = p[sx] * (1. - fz) + p[sx + 1] * fz;
    double c11 = p[sx + sy] * (1. - fz) + p[sx + sy + 1] * fz;
    double c0 = c00 * (1. - fy) + c01 * fy;
    double c1 = c10 * (1. - fy) + c11 * fy;
    return c0 * (1. - fx) + c1 * fx;
  }
};

//! Separable Gaussian smoothing (truncated at 4 sigma, zero outside)
void gaussian_smooth(Grid3 &g, double sigma) {
  int radius = static_cast<int>(4. * sigma + 0.5);
  std::vector<double> kernel(2 * radius + 1);
  double sum = 0.;
  for (int i = -radius; i <= radius; ++i) {
    kernel[i + radius] = std::exp(-0.5 * i * i / (sigma * sigma));
    sum += kernel[i + radius];
  }
  for (double &k : kernel) k /= sum;
  for (int axis = 0; axis < 3; ++axis) {
    int n = g.dim[axis];
    int a1 = (axis + 1) % 3, a2 = (axis + 2) % 3;
    std::vector<double> line(n), out(n);
    int idx[3];
    for (idx[a1] = 0; idx[a1] < g.dim[a1]; ++idx[a1]) {
      for (idx[a2] = 0; idx[a2] < g.dim[a2]; ++idx[a2]) {
        for (idx[axis] = 0; idx[axis] < n; ++idx[axis]) {
          line[idx[axis]] = g(idx[0], idx[1], idx[2]);
        }
        for (int i = 0; i < n; ++i) {
          double s = 0.;
          int lo = std::max(0, i - radius), hi = std::min(n - 1, i + radius);
          for (int j = lo; j <= hi; ++j) {
            s += kernel[j - i + radius] * line[j];
          }
          out[i] = s;
        }
        for (idx[axis] = 0; idx[axis] < n; ++idx[axis]) {
          g(idx[0], idx[1], idx[2]) = out[idx[axis]];
        }
      }
    }
  }
}

struct Candidate {
  double score;
  V3 u, t;
  Candidate() : score(0.) {}
  Candidate(double score, const V3 &u, const V3 &t)
      : score(score), u(u), t(t) {}
};

bool operator<(const Candidate &a, const Candidate &b) {
  // sort by decreasing score
  return a.score > b.score;
}

//! Minimize a function of 4 variables with the Nelder-Mead simplex method
template <class F>
std::vector<double> nelder_mead(F f, const std::vector<std::vector<double> >
                                         &initial_simplex,
                                double xatol, double fatol,
                                unsigned max_evaluations) {
  const unsigned N = initial_simplex[0].size();
  const double rho = 1., chi = 2., psi = 0.5, sigma = 0.5;
  std::vector<std::vector<double> > sim = initial_simplex;
  std::vector<double> fsim(N + 1);
  unsigned nfev = 0;
  for (unsigned i = 0; i <= N; ++i) {
    fsim[i] = f(sim[i]);
    ++nfev;
  }
  std::vector<unsigned> order(N + 1);
  auto sort_simplex = [&]() {
    for (unsigned i = 0; i <= N; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](unsigned a, unsigned b) {
      return fsim[a] < fsim[b];
    });
    std::vector<std::vector<double> > s2(N + 1);
    std::vector<double> f2(N + 1);
    for (unsigned i = 0; i <= N; ++i) {
      s2[i] = sim[order[i]];
      f2[i] = fsim[order[i]];
    }
    sim.swap(s2);
    fsim.swap(f2);
  };
  auto combine = [&](const std::vector<double> &xbar, double a,
                     const std::vector<double> &x) {
    std::vector<double> r(N);
    for (unsigned j = 0; j < N; ++j) r[j] = (1. + a) * xbar[j] - a * x[j];
    return r;
  };
  sort_simplex();
  while (nfev < max_evaluations) {
    double maxx = 0., maxf = 0.;
    for (unsigned i = 1; i <= N; ++i) {
      maxf = std::max(maxf, std::abs(fsim[0] - fsim[i]));
      for (unsigned j = 0; j < N; ++j) {
        maxx = std::max(maxx, std::abs(sim[i][j] - sim[0][j]));
      }
    }
    if (maxx <= xatol && maxf <= fatol) break;
    std::vector<double> xbar(N, 0.);
    for (unsigned i = 0; i < N; ++i) {
      for (unsigned j = 0; j < N; ++j) xbar[j] += sim[i][j] / N;
    }
    std::vector<double> xr = combine(xbar, rho, sim[N]);
    double fxr = f(xr);
    ++nfev;
    bool shrink = false;
    if (fxr < fsim[0]) {
      std::vector<double> xe = combine(xbar, rho * chi, sim[N]);
      double fxe = f(xe);
      ++nfev;
      if (fxe < fxr) {
        sim[N] = xe;
        fsim[N] = fxe;
      } else {
        sim[N] = xr;
        fsim[N] = fxr;
      }
    } else if (fxr < fsim[N - 1]) {
      sim[N] = xr;
      fsim[N] = fxr;
    } else if (fxr < fsim[N]) {
      std::vector<double> xc = combine(xbar, psi * rho, sim[N]);
      double fxc = f(xc);
      ++nfev;
      if (fxc <= fxr) {
        sim[N] = xc;
        fsim[N] = fxc;
      } else {
        shrink = true;
      }
    } else {
      std::vector<double> xcc = combine(xbar, -psi, sim[N]);
      double fxcc = f(xcc);
      ++nfev;
      if (fxcc < fsim[N]) {
        sim[N] = xcc;
        fsim[N] = fxcc;
      } else {
        shrink = true;
      }
    }
    if (shrink) {
      for (unsigned i = 1; i <= N; ++i) {
        for (unsigned j = 0; j < N; ++j) {
          sim[i][j] = sim[0][j] + sigma * (sim[i][j] - sim[0][j]);
        }
        fsim[i] = f(sim[i]);
        ++nfev;
      }
    }
    sort_simplex();
  }
  return sim[0];
}

class CnDocker {
  int cn_;
  double angle_;
  ContactPotential pot_;
  CnDockingParameters params_;
  V3 centroid_;
  algebra::Vector3Ds x_;  // monomer atoms, centered at the centroid
  double radius_;
  unsigned ring_stride_;
  IMP::PointerMember<algebra::NearestNeighbor3D> nn_;
  // Fine grid of the contact potential
  Grid3 fine_;
  V3 fine_lo_;
  // Fourier transform of the periodic coarse grid of the contact potential
  int n_;
  multifit::internal::FFTWGrid<double> real_grid_;
  multifit::internal::FFTWGrid<fftw_complex> coarse_fft_, lig_fft_;
  multifit::internal::FFTWPlan plan_r2c_, plan_c2r_;

  void setup_fine_grid();
  void setup_coarse_grid();
  void splat(const algebra::Vector3Ds &pts);

  double lookup(const V3 &pt) const {
    V3 g = (pt - fine_lo_) / params_.fine_spacing;
    return fine_.interpolate(g[0], g[1], g[2]);
  }

  //! Get the point on the symmetry axis closest to the monomer centroid
  V3 get_axis_point(const V3 &u, const V3 &t, const Mat3 &R1) const {
    // solve (I - R1) p = t for p in the plane perpendicular to u
    V3 e1, e2;
    get_plane_basis(u, e1, e2);
    V3 c1 = e1 - R1 * e1, c2 = e2 - R1 * e2;
    double a11 = c1 * e1, a12 = c2 * e1, a21 = c1 * e2, a22 = c2 * e2;
    double b1 = t * e1, b2 = t * e2;
    double det = a11 * a22 - a12 * a21;
    double q1 = (b1 * a22 - a12 * b2) / det;
    double q2 = (a11 * b2 - a21 * b1) / det;
    return q1 * e1 + q2 * e2;
  }

  std::vector<Candidate> coarse_search();
  Candidate refine(const Candidate &c);
  std::vector<Candidate> cluster(const std::vector<Candidate> &sols) const;
  Ints get_interface_atoms(const V3 &u, const V3 &t) const;

 public:
  CnDocker(const algebra::Vector3Ds &coords, int cn, bool ca_only,
           const CnDockingParameters &params);

  //! Interface score of the ring with dock transformation (u, t)
  /** If atoms is non-null, only those monomer atoms are used for the
      neighbor interface (other atoms are assumed to score 0). */
  double score(const V3 &u, const V3 &t, const Ints *atoms = nullptr) const;

  std::vector<Candidate> dock();

  algebra::Transformation3D get_transformation(const V3 &u,
                                               const V3 &t) const;
};

CnDocker::CnDocker(const algebra::Vector3Ds &coords, int cn, bool ca_only,
                   const CnDockingParameters &params)
    : cn_(cn), angle_(2. * PI / cn), pot_(ca_only), params_(params) {
  IMP_USAGE_CHECK(cn >= 2, "Symmetry degree must be at least 2");
  IMP_USAGE_CHECK(!coords.empty(), "No monomer coordinates given");
  centroid_ = V3(0., 0., 0.);
  for (const V3 &c : coords) centroid_ += c;
  centroid_ /= coords.size();
  radius_ = 0.;
  for (const V3 &c : coords) {
    x_.push_back(c - centroid_);
    radius_ = std::max(radius_, x_.back().get_magnitude());
  }
  if (params_.coarse_spacing <= 0.) {
    // finer grid for small subunits (cheap, and their interfaces
    // are a larger fraction of the surface)
    params_.coarse_spacing = radius_ < 35. ? 2.0 : 2.5;
  }
  if (params_.angle_step <= 0.) {
    // angular step such that the furthest atom moves ~1.2 voxels
    params_.angle_step = std::max(
        3.5, std::min(5., 180. / PI * 1.2 * params_.coarse_spacing /
                              std::max(radius_, 1.)));
  }
  ring_stride_ = std::max(static_cast<size_t>(1), x_.size() / 500);
  nn_ = new algebra::NearestNeighbor3D(x_);
  setup_fine_grid();
  setup_coarse_grid();
}

void CnDocker::setup_fine_grid() {
  double s = params_.fine_spacing;
  double pad = pot_.get_cutoff() + 2. * s;
  algebra::BoundingBox3D bb(x_);
  fine_lo_ = bb.get_corner(0) - V3(pad, pad, pad);
  V3 hi = bb.get_corner(1) + V3(pad, pad, pad);
  int dim[3];
  for (int i = 0; i < 3; ++i) {
    dim[i] = static_cast<int>(std::ceil((hi[i] - fine_lo_[i]) / s)) + 1;
  }
  // squared distance (in voxels) to the nearest atom-occupied voxel
  const double big = 1e20;
  std::vector<double> d2(static_cast<size_t>(dim[0]) * dim[1] * dim[2], big);
  auto index = [&](int i, int j, int k) {
    return (static_cast<size_t>(i) * dim[1] + j) * dim[2] + k;
  };
  for (const V3 &x : x_) {
    int i[3];
    for (int k = 0; k < 3; ++k) {
      i[k] = static_cast<int>(std::round((x[k] - fine_lo_[k]) / s));
    }
    d2[index(i[0], i[1], i[2])] = 0.;
  }
  int maxdim = std::max(dim[0], std::max(dim[1], dim[2]));
  std::vector<double> f(maxdim), d(maxdim), z(maxdim + 1);
  std::vector<int> v(maxdim);
  for (int axis = 0; axis < 3; ++axis) {
    int n = dim[axis];
    int a1 = (axis + 1) % 3, a2 = (axis + 2) % 3;
    f.resize(n);
    d.resize(n);
    int idx[3];
    for (idx[a1] = 0; idx[a1] < dim[a1]; ++idx[a1]) {
      for (idx[a2] = 0; idx[a2] < dim[a2]; ++idx[a2]) {
        for (idx[axis] = 0; idx[axis] < n; ++idx[axis]) {
          f[idx[axis]] = d2[index(idx[0], idx[1], idx[2])];
        }
        distance_transform_1d(f, d, v, z);
        for (idx[axis] = 0; idx[axis] < n; ++idx[axis]) {
          d2[index(idx[0], idx[1], idx[2])] = d[idx[axis]];
        }
      }
    }
  }
  fine_.resize(dim[0], dim[1], dim[2], 0.);
  for (size_t i = 0; i < d2.size(); ++i) {
    fine_.data[i] = pot_(std::sqrt(d2[i]) * s);
  }
}

void CnDocker::setup_coarse_grid() {
  double s = params_.coarse_spacing;
  // neighbor translations can be up to 2*radius+cutoff; grid must hold
  // receptor + ligand without wraparound
  double extent = 2. * (2. * radius_ + pot_.get_cutoff()) + 2. * s;
  n_ = get_fft_friendly_size(static_cast<int>(std::ceil(extent / s)));
  size_t nreal = static_cast<size_t>(n_) * n_ * n_;
  size_t ncomplex = static_cast<size_t>(n_) * n_ * (n_ / 2 + 1);
  real_grid_.resize(nreal);
  coarse_fft_.resize(ncomplex);
  lig_fft_.resize(ncomplex);
  plan_r2c_ = fftw_plan_dft_r2c_3d(n_, n_, n_, real_grid_, lig_fft_,
                                   FFTW_MEASURE);
  plan_c2r_ = fftw_plan_dft_c2r_3d(n_, n_, n_, lig_fft_, real_grid_,
                                   FFTW_MEASURE);

  // potential at coarse grid points: average of the fine potential
  // over the coarse voxel (Gaussian smoothing approximation)
  Grid3 smooth = fine_;
  gaussian_smooth(smooth, 0.5 * s / params_.fine_spacing);
  for (int i = 0; i < n_; ++i) {
    double x = (i >= n_ / 2 ? i - n_ : i) * s;
    for (int j = 0; j < n_; ++j) {
      double y = (j >= n_ / 2 ? j - n_ : j) * s;
      for (int k = 0; k < n_; ++k) {
        double z = (k >= n_ / 2 ? k - n_ : k) * s;
        V3 g = (V3(x, y, z) - fine_lo_) / params_.fine_spacing;
        real_grid_[(static_cast<size_t>(i) * n_ + j) * n_ + k] =
            smooth.interpolate(g[0], g[1], g[2]);
      }
    }
  }
  fftw_execute_dft_r2c(plan_r2c_.get(), real_grid_, coarse_fft_);
  IMP_LOG_TERSE("Cn docking: coarse grid " << n_ << "^3 at " << s
                << "A, fine grid " << fine_.dim[0] << "x" << fine_.dim[1]
                << "x" << fine_.dim[2] << " at " << params_.fine_spacing
                << "A, monomer radius " << radius_ << "A" << std::endl);
}

void CnDocker::splat(const algebra::Vector3Ds &pts) {
  size_t nreal = static_cast<size_t>(n_) * n_ * n_;
  double *grid = real_grid_;
  std::fill(grid, grid + nreal, 0.);
  double s = params_.coarse_spacing;
  for (const V3 &p : pts) {
    int f[3];
    double w[3];
    for (int k = 0; k < 3; ++k) {
      double g = p[k] / s;
      double fl = std::floor(g);
      w[k] = g - fl;
      // wrap into [0, n)
      f[k] = ((static_cast<int>(fl) % n_) + n_) % n_;
    }
    for (int dx = 0; dx < 2; ++dx) {
      double wx = dx ? w[0] : 1. - w[0];
      size_t ix = (f[0] + dx) % n_;
      for (int dy = 0; dy < 2; ++dy) {
        double wy = dy ? w[1] : 1. - w[1];
        size_t iy = (f[1] + dy) % n_;
        for (int dz = 0; dz < 2; ++dz) {
          double wz = dz ? w[2] : 1. - w[2];
          size_t iz = (f[2] + dz) % n_;
          grid[(ix * n_ + iy) * n_ + iz] += wx * wy * wz;
        }
      }
    }
  }
}

std::vector<Candidate> CnDocker::coarse_search() {
  algebra::Vector3Ds dirs = get_hemisphere_directions(params_.angle_step);
  IMP_LOG_TERSE("Cn docking: coarse search over " << dirs.size()
                << " axis directions (" << params_.angle_step << " deg)"
                << std::endl);
  double s = params_.coarse_spacing;
  double max_t = 2. * radius_ + pot_.get_cutoff();
  int m = static_cast<int>(std::ceil(max_t / s));
  int nl = 2 * m + 1;
  size_t ncomplex = static_cast<size_t>(n_) * n_ * (n_ / 2 + 1);
  std::vector<double> vals(nl * nl);
  std::vector<Candidate> cands;
  algebra::Vector3Ds lig(x_.size());
  const double *corr = real_grid_;
  for (const V3 &u : dirs) {
    Mat3 R = get_rotation_matrix(u, angle_);
    for (unsigned i = 0; i < x_.size(); ++i) lig[i] = R * x_[i];
    splat(lig);
    fftw_execute_dft_r2c(plan_r2c_.get(), real_grid_, lig_fft_);
    // correlation: F(receptor) * conj(F(ligand))
    for (size_t i = 0; i < ncomplex; ++i) {
      double ar = coarse_fft_[i][0], ai = coarse_fft_[i][1];
      double br = lig_fft_[i][0], bi = lig_fft_[i][1];
      lig_fft_[i][0] = ar * br + ai * bi;
      lig_fft_[i][1] = ai * br - ar * bi;
    }
    fftw_execute_dft_c2r(plan_c2r_.get(), lig_fft_, real_grid_);

    // sample the correlation on the plane perpendicular to u
    V3 e1, e2;
    get_plane_basis(u, e1, e2);
    for (int a = 0; a < nl; ++a) {
      double la = (a - m) * s;
      for (int b = 0; b < nl; ++b) {
        double lb = (b - m) * s;
        double &val = vals[a * nl + b];
        if (la * la + lb * lb > max_t * max_t) {
          val = -1e30;
          continue;
        }
        V3 g = (la * e1 + lb * e2) / s;
        int f[3];
        double w[3];
        for (int k = 0; k < 3; ++k) {
          double fl = std::floor(g[k]);
          w[k] = g[k] - fl;
          f[k] = ((static_cast<int>(fl) % n_) + n_) % n_;
        }
        val = 0.;
        for (int dx = 0; dx < 2; ++dx) {
          double wx = dx ? w[0] : 1. - w[0];
          size_t ix = (f[0] + dx) % n_;
          for (int dy = 0; dy < 2; ++dy) {
            double wy = dy ? w[1] : 1. - w[1];
            size_t iy = (f[1] + dy) % n_;
            for (int dz = 0; dz < 2; ++dz) {
              double wz = dz ? w[2] : 1. - w[2];
              size_t iz = (f[2] + dz) % n_;
              val += wx * wy * wz * corr[(ix * n_ + iy) * n_ + iz];
            }
          }
        }
      }
    }
    // find local maxima with positive score
    std::vector<std::pair<double, int> > peaks;
    for (int a = 0; a < nl; ++a) {
      for (int b = 0; b < nl; ++b) {
        double v = vals[a * nl + b];
        if (v <= 0.) continue;
        bool is_peak = true;
        for (int da = -1; da <= 1 && is_peak; ++da) {
          for (int db = -1; db <= 1; ++db) {
            int a2 = a + da, b2 = b + db;
            if (a2 < 0 || b2 < 0 || a2 >= nl || b2 >= nl) continue;
            if (vals[a2 * nl + b2] > v) {
              is_peak = false;
              break;
            }
          }
        }
        if (is_peak) peaks.push_back(std::make_pair(v, a * nl + b));
      }
    }
    unsigned npeak = std::min(static_cast<size_t>(params_.peaks_per_direction),
                              peaks.size());
    std::partial_sort(peaks.begin(), peaks.begin() + npeak, peaks.end(),
                      [](const std::pair<double, int> &p1,
                         const std::pair<double, int> &p2) {
                        return p1.first > p2.first;
                      });
    for (unsigned i = 0; i < npeak; ++i) {
      int a = peaks[i].second / nl, b = peaks[i].second % nl;
      cands.push_back(Candidate(peaks[i].first, u,
                                (a - m) * s * e1 + (b - m) * s * e2));
    }
  }
  std::stable_sort(cands.begin(), cands.end());
  return cands;
}

double CnDocker::score(const V3 &u, const V3 &t, const Ints *atoms) const {
  Mat3 R = get_rotation_matrix(u, angle_);
  V3 p = get_axis_point(u, t, R);
  double sc = 0.;
  if (atoms) {
    for (int i : *atoms) sc += lookup(R * (x_[i] - p) + p);
  } else {
    for (const V3 &x : x_) sc += lookup(R * (x - p) + p);
  }
  if (cn_ >= 4) {
    // non-neighbors should not clash (only negative terms count);
    // use a subset of atoms for speed
    for (int k = 2; k <= cn_ / 2; ++k) {
      Mat3 Rk = get_rotation_matrix(u, angle_ * k);
      double neg = 0.;
      for (unsigned i = 0; i < x_.size(); i += ring_stride_) {
        double v = lookup(Rk * (x_[i] - p) + p);
        if (v < 0.) neg += v;
      }
      // pairs k and cn-k are equivalent, except when k == cn/2
      sc += neg * ring_stride_ * (2 * k < cn_ ? 2 : 1);
    }
  }
  return sc;
}

Ints CnDocker::get_interface_atoms(const V3 &u, const V3 &t) const {
  const double margin = 4.0;
  double cut2 = (pot_.get_cutoff() + margin) * (pot_.get_cutoff() + margin);
  Mat3 R = get_rotation_matrix(u, angle_);
  V3 p = get_axis_point(u, t, R);
  Ints ret;
  for (unsigned i = 0; i < x_.size(); ++i) {
    V3 nb = R * (x_[i] - p) + p;
    unsigned j = nn_->get_nearest_neighbor(nb);
    if (algebra::get_squared_distance(nb, x_[j]) <= cut2) {
      ret.push_back(i);
    }
  }
  return ret;
}

Candidate CnDocker::refine(const Candidate &c) {
  const V3 &u = c.u;
  V3 e1, e2;
  get_plane_basis(u, e1, e2);
  V3 p0 = get_axis_point(u, c.t, get_rotation_matrix(u, angle_));
  Ints atoms = get_interface_atoms(u, c.t);
  if (atoms.empty()) {
    return Candidate(score(u, c.t), u, c.t);
  }
  double angle = angle_;
  // parameters: change in axis direction (2), change in axis position (2)
  auto unpack = [&](const std::vector<double> &v, V3 &uu, V3 &tt) {
    uu = (u + v[0] * e1 + v[1] * e2).get_unit_vector();
    V3 p = p0 + v[2] * e1 + v[3] * e2;
    p -= (p * uu) * uu;
    tt = p - get_rotation_matrix(uu, angle) * p;
  };
  auto f = [&](const std::vector<double> &v) {
    V3 uu, tt;
    unpack(v, uu, tt);
    return -score(uu, tt, &atoms);
  };
  double step = params_.angle_step * PI / 180. / 2.;
  std::vector<std::vector<double> > simplex = {{0., 0., 0., 0.},
                                               {step, 0., 0., 0.},
                                               {0., step, 0., 0.},
                                               {0., 0., 1., 0.},
                                               {0., 0., 0., 1.}};
  std::vector<double> best = nelder_mead(f, simplex, 1e-3, 0.5,
                                         params_.refine_max_evaluations);
  V3 uu, tt;
  unpack(best, uu, tt);
  return Candidate(score(uu, tt), uu, tt);
}

std::vector<Candidate> CnDocker::cluster(
    const std::vector<Candidate> &sols) const {
  // Greedy clustering (sols sorted by decreasing score) by the RMSD of the
  // placed neighbor (T or T^-1 give the same ring)
  unsigned step = std::max(static_cast<size_t>(1), x_.size() / 100);
  algebra::Vector3Ds rep;
  for (unsigned i = 0; i < x_.size(); i += step) rep.push_back(x_[i]);
  double thr = params_.cluster_rmsd * params_.cluster_rmsd * rep.size();
  std::vector<algebra::Vector3Ds> kept_pos;
  std::vector<Candidate> ret;
  for (const Candidate &c : sols) {
    if (c.score <= 0.) continue;
    Mat3 R = get_rotation_matrix(c.u, angle_);
    Mat3 Rinv = R.get_transpose();
    V3 p = get_axis_point(c.u, c.t, R);
    algebra::Vector3Ds pos(rep.size()), posinv(rep.size());
    for (unsigned i = 0; i < rep.size(); ++i) {
      pos[i] = R * (rep[i] - p) + p;
      posinv[i] = Rinv * (rep[i] - p) + p;
    }
    bool redundant = false;
    for (const algebra::Vector3Ds &k : kept_pos) {
      double d1 = 0., d2 = 0.;
      for (unsigned i = 0; i < rep.size(); ++i) {
        d1 += algebra::get_squared_distance(k[i], pos[i]);
        d2 += algebra::get_squared_distance(k[i], posinv[i]);
      }
      if (d1 < thr || d2 < thr) {
        redundant = true;
        break;
      }
    }
    if (!redundant) {
      kept_pos.push_back(pos);
      ret.push_back(c);
    }
  }
  return ret;
}

std::vector<Candidate> CnDocker::dock() {
  std::vector<Candidate> cands = coarse_search();
  IMP_LOG_TERSE("Cn docking: " << cands.size() << " FFT candidates"
                << std::endl);
  // rescore on the fine grid
  if (cands.size() > params_.n_rescore) {
    cands.erase(cands.begin() + params_.n_rescore, cands.end());
  }
  for (Candidate &c : cands) c.score = score(c.u, c.t);
  std::stable_sort(cands.begin(), cands.end());
  // refine the best
  unsigned nrefine = std::min(static_cast<size_t>(params_.n_refine),
                              cands.size());
  for (unsigned i = 0; i < nrefine; ++i) {
    cands[i] = refine(cands[i]);
  }
  std::stable_sort(cands.begin(), cands.end());
  std::vector<Candidate> ret = cluster(cands);
  IMP_LOG_TERSE("Cn docking: " << ret.size() << " clustered solutions"
                << std::endl);
  return ret;
}

algebra::Transformation3D CnDocker::get_transformation(const V3 &u,
                                                       const V3 &t) const {
  // In the original (uncentered) frame, x -> R (x - c) + c + t
  Mat3 R = get_rotation_matrix(u, angle_);
  algebra::Rotation3D rot = algebra::get_rotation_about_axis(u, angle_);
  return algebra::Transformation3D(rot, centroid_ + t - R * centroid_);
}

}  // namespace

std::vector<CnDockingSolution> dock_cn(const algebra::Vector3Ds &coords,
                                       int cn, bool ca_only,
                                       const CnDockingParameters &params) {
  CnDocker docker(coords, cn, ca_only, params);
  std::vector<Candidate> sols = docker.dock();
  std::vector<CnDockingSolution> ret;
  for (const Candidate &c : sols) {
    if (ret.size() >= params.max_solutions) break;
    ret.push_back(CnDockingSolution(c.score,
                                    docker.get_transformation(c.u, c.t)));
  }
  return ret;
}

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE
