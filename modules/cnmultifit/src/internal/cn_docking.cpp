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
    monomer atom: negative for clashes, +1 for contacts, and 0 beyond
    the cutoff. Surface clashes are kept moderate and amplified by burial
    depth inside the molecular surface so deep core penetration is strongly
    penalized while surface side-chain flexibility / noise is tolerated. */
class ContactPotential {
  std::vector<std::pair<double, double> > knots_;

 public:
  ContactPotential(bool ca_only) {
    if (ca_only) {
      // One point per residue; inter-subunit CA-CA contacts are ~4.8-9.5A
      knots_ = {{0., -8.}, {3.4, -6.}, {4.2, -1.}, {4.8, 1.},
                {8.8, 1.}, {10.0, 0.}};
    } else {
      // Heavy atoms; contacts are ~2.6-4.6A with a firm steric wall
      knots_ = {{0., -5.}, {1.2, -4.5}, {1.8, -2.5}, {2.35, 0.},
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

//! Exact 3D squared Euclidean distance transform in-place
void distance_transform_3d(std::vector<double> &d2, const int dim[3]) {
  auto index = [&](int i, int j, int k) {
    return (static_cast<size_t>(i) * dim[1] + j) * dim[2] + k;
  };
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

//! Separable Gaussian convolution (truncated at 3 sigma, zero outside).
/** When normalize is true, acts as a smoothing filter; when false, computes
    the unnormalized sum of exp(-r^2 / (2*sigma^2)) over splatted atoms. */
void gaussian_convolve(Grid3 &g, double sigma, bool normalize) {
  if (sigma <= 0.) return;
  int radius = static_cast<int>(3.0 * sigma + 0.5);
  if (radius < 1) return;
  std::vector<double> kernel(2 * radius + 1);
  double sum = 0.;
  for (int i = -radius; i <= radius; ++i) {
    kernel[i + radius] = std::exp(-0.5 * i * i / (sigma * sigma));
    sum += kernel[i + radius];
  }
  if (normalize) {
    for (double &k : kernel) k /= sum;
  }
  for (int axis = 0; axis < 3; ++axis) {
    int n = g.dim[axis];
    std::vector<double> line(n), out(n);
    if (axis == 2) {
      // z varies fastest: operate directly on contiguous rows
      for (int i0 = 0; i0 < g.dim[0]; ++i0) {
        for (int i1 = 0; i1 < g.dim[1]; ++i1) {
          float *row = &g.data[g.index(i0, i1, 0)];
          for (int i = 0; i < n; ++i) {
            double s = 0.;
            int lo = std::max(0, i - radius), hi = std::min(n - 1, i + radius);
            for (int j = lo; j <= hi; ++j) {
              s += kernel[j - i + radius] * row[j];
            }
            out[i] = s;
          }
          for (int i = 0; i < n; ++i) row[i] = static_cast<float>(out[i]);
        }
      }
    } else {
      int a1 = (axis + 1) % 3, a2 = (axis + 2) % 3;
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
            g(idx[0], idx[1], idx[2]) = static_cast<float>(out[idx[axis]]);
          }
        }
      }
    }
  }
}

struct Candidate {
  double score;
  double coarse_score;
  V3 u, t;
  Candidate() : score(0.), coarse_score(0.) {}
  Candidate(double score, double coarse_score, const V3 &u, const V3 &t)
      : score(score), coarse_score(coarse_score), u(u), t(t) {}
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
  bool ca_only_;
  double angle_;
  ContactPotential pot_;
  CnDockingParameters params_;
  V3 centroid_;
  algebra::Vector3Ds x_;  // monomer atoms, centered at the centroid
  double radius_;
  unsigned ring_stride_;
  unsigned coarse_ring_stride_;
  IMP::PointerMember<algebra::NearestNeighbor3D> nn_;
  // Fine contact potential grid and a mildly softened grid for pre-refinement
  // rescoring and broad basin capture
  Grid3 fine_, soft_;
  V3 fine_lo_;
  // Fourier transform of the periodic coarse grid of the contact potential
  int n_;
  multifit::internal::FFTWGrid<double> real_grid_;
  multifit::internal::FFTWGrid<fftw_complex> coarse_fft_, lig_fft_;
  multifit::internal::FFTWPlan plan_r2c_, plan_c2r_;

  void setup_fine_grid();
  void setup_coarse_grid();
  void splat(const algebra::Vector3Ds &pts);

  double lookup(const Grid3 &g, const V3 &pt) const {
    V3 gc = (pt - fine_lo_) / params_.fine_spacing;
    return g.interpolate(gc[0], gc[1], gc[2]);
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

  double quick_non_neighbor_clash(const V3 &u, const V3 &t) const;
  std::vector<Candidate> coarse_search();
  Candidate refine(const Candidate &c);
  std::vector<Candidate> cluster(const std::vector<Candidate> &sols,
                                 double rmsd_thr, bool keep_nonpos) const;
  void get_interface_atoms(const V3 &u, const V3 &t, double margin,
                           Ints &fwd_atoms, Ints &rev_atoms,
                           V3 &if_center) const;

 public:
  CnDocker(const algebra::Vector3Ds &coords, int cn, bool ca_only,
           const CnDockingParameters &params);

  //! Symmetric interface score of the ring with dock transformation (u, t)
  /** Evaluates both T and T^-1 (for cn >= 3) so the score is invariant to
      axis orientation (+u vs -u), and adds a clash penalty against
      non-neighboring subunits for cn >= 4. */
  double score(const Grid3 &g, const V3 &u, const V3 &t,
               const Ints *fwd_atoms = nullptr,
               const Ints *rev_atoms = nullptr) const;

  std::vector<Candidate> dock();

  algebra::Transformation3D get_transformation(const V3 &u,
                                               const V3 &t) const;
};

CnDocker::CnDocker(const algebra::Vector3Ds &coords, int cn, bool ca_only,
                   const CnDockingParameters &params)
    : cn_(cn), ca_only_(ca_only), angle_(2. * PI / cn), pot_(ca_only),
      params_(params) {
  IMP_USAGE_CHECK(cn >= 2, "Symmetry degree must be at least 2");
  IMP_USAGE_CHECK(!coords.empty(), "No monomer coordinates given");
  IMP_USAGE_CHECK(params_.fine_spacing > 0., "fine_spacing must be positive");
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
    // angular step such that the furthest atom moves ~1.2 voxels;
    // tighten slightly for C2 where the rotation leverage 2*sin(pi/2)=2
    // is largest
    double sym_factor = (cn_ == 2) ? 1.25 : 1.0;
    params_.angle_step = std::max(
        3.5, std::min(5.0 / sym_factor,
                      180. / PI * 1.2 * params_.coarse_spacing /
                          (std::max(radius_, 1.) * sym_factor)));
  }
  ring_stride_ = std::max(static_cast<size_t>(1), x_.size() / 500);
  coarse_ring_stride_ = std::max(static_cast<size_t>(1), x_.size() / 40);
  nn_ = new algebra::NearestNeighbor3D(x_);
  setup_fine_grid();
  setup_coarse_grid();
}

void CnDocker::setup_fine_grid() {
  double s = params_.fine_spacing;
  double pad = pot_.get_cutoff() + 3. * s;
  algebra::BoundingBox3D bb(x_);
  fine_lo_ = bb.get_corner(0) - V3(pad, pad, pad);
  V3 hi = bb.get_corner(1) + V3(pad, pad, pad);
  int dim[3];
  for (int i = 0; i < 3; ++i) {
    dim[i] = static_cast<int>(std::ceil((hi[i] - fine_lo_[i]) / s)) + 1;
  }
  size_t ntot = static_cast<size_t>(dim[0]) * dim[1] * dim[2];
  // 1) Squared distance (in voxels) to the nearest atom-occupied voxel
  const double big = 1e20;
  std::vector<double> d2(ntot, big);
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
  distance_transform_3d(d2, dim);

  // 2) Burial depth inside the molecular surface (distance from exterior)
  double r_occ = ca_only_ ? 3.8 : 2.3;
  double r_occ2_vox = (r_occ / s) * (r_occ / s);
  std::vector<double> depth2(ntot, big);
  for (size_t i = 0; i < ntot; ++i) {
    if (d2[i] > r_occ2_vox) depth2[i] = 0.;
  }
  distance_transform_3d(depth2, dim);

  // 3) Smooth local atom density rho(r) = sum_j exp(-|r - x_j|^2 / (2*sig^2))
  //    to favor well-packed complementary interfaces over grazing contacts
  Grid3 rho;
  rho.resize(dim[0], dim[1], dim[2], 0.);
  for (const V3 &x : x_) {
    int f[3];
    double w[3];
    for (int k = 0; k < 3; ++k) {
      double g = (x[k] - fine_lo_[k]) / s;
      f[k] = std::max(0, std::min(dim[k] - 2, static_cast<int>(std::floor(g))));
      w[k] = g - f[k];
    }
    for (int dx = 0; dx < 2; ++dx) {
      double wx = dx ? w[0] : 1. - w[0];
      for (int dy = 0; dy < 2; ++dy) {
        double wy = dy ? w[1] : 1. - w[1];
        for (int dz = 0; dz < 2; ++dz) {
          double wz = dz ? w[2] : 1. - w[2];
          rho(f[0] + dx, f[1] + dy, f[2] + dz) +=
              static_cast<float>(wx * wy * wz);
        }
      }
    }
  }
  double rho_sig = ca_only_ ? 3.5 : 2.1;
  gaussian_convolve(rho, rho_sig / s, false);

  // Combine distance, burial depth and local contact density into fine_
  fine_.resize(dim[0], dim[1], dim[2], 0.);
  double density_alpha = ca_only_ ? 0.35 : 0.25;
  double depth_start = ca_only_ ? 1.5 : 1.0;
  double depth_slope = ca_only_ ? 2.5 : 3.5;
  for (size_t i = 0; i < ntot; ++i) {
    double d = std::sqrt(d2[i]) * s;
    double dep = std::sqrt(depth2[i]) * s;
    double v = pot_(d);
    if (v > 0.) {
      v *= (1.0 + density_alpha * std::min(static_cast<double>(rho.data[i]), 2.0));
    } else if (dep > depth_start) {
      v = std::max(-12., v - depth_slope * (dep - depth_start));
    }
    fine_.data[i] = static_cast<float>(v);
  }

  // Mildly softened potential grid for pre-refinement rescoring and
  // broad-basin first-pass refinement; clip deep-core negatives first so
  // the core does not bleed into and wipe out the positive contact shell
  soft_ = fine_;
  for (float &v : soft_.data) {
    if (v < -5.f) v = -5.f;
  }
  gaussian_convolve(soft_, 0.75 / s, true);
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

  // Coarse potential: start from clipped/smoothed soft_ (sigma = 0.75A) and
  // apply only the residual Gaussian smoothing needed to reach 0.4 * s
  double target_sig = 0.4 * s;
  Grid3 smooth = soft_;
  if (target_sig > 0.75) {
    double rem_sig = std::sqrt(target_sig * target_sig - 0.75 * 0.75);
    gaussian_convolve(smooth, rem_sig / params_.fine_spacing, true);
  }
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

double CnDocker::quick_non_neighbor_clash(const V3 &u, const V3 &t) const {
  if (cn_ < 4) return 0.;
  Mat3 R1 = get_rotation_matrix(u, angle_);
  V3 p = get_axis_point(u, t, R1);
  double total_neg = 0.;
  for (int k = 2; k <= cn_ / 2; ++k) {
    Mat3 Rk = get_rotation_matrix(u, angle_ * k);
    double neg = 0.;
    for (unsigned i = 0; i < x_.size(); i += coarse_ring_stride_) {
      double v = lookup(soft_, Rk * (x_[i] - p) + p);
      if (v < 0.) neg += v;
    }
    total_neg += neg * coarse_ring_stride_ * (2 * k < cn_ ? 2 : 1);
  }
  return total_neg;
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
  size_t nreal = static_cast<size_t>(n_) * n_ * n_;
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
    // Inspect top raw peaks, filter severely self-clashing rings (cn >= 4),
    // and refine peak position with sub-voxel parabolic interpolation
    unsigned raw_check = std::min(
        static_cast<size_t>(params_.peaks_per_direction * 4), peaks.size());
    std::partial_sort(peaks.begin(), peaks.begin() + raw_check, peaks.end(),
                      [](const std::pair<double, int> &p1,
                         const std::pair<double, int> &p2) {
                        return p1.first > p2.first;
                      });
    unsigned kept = 0;
    for (unsigned i = 0; i < raw_check && kept < params_.peaks_per_direction;
         ++i) {
      int a = peaks[i].second / nl, b = peaks[i].second % nl;
      double da = 0., db = 0.;
      if (a > 0 && a < nl - 1) {
        double vm = vals[(a - 1) * nl + b];
        double v0 = vals[a * nl + b];
        double vp = vals[(a + 1) * nl + b];
        double den = vm - 2. * v0 + vp;
        if (den < -1e-9 && vm > -1e20 && vp > -1e20) {
          da = std::max(-0.5, std::min(0.5, 0.5 * (vm - vp) / den));
        }
      }
      if (b > 0 && b < nl - 1) {
        double vm = vals[a * nl + b - 1];
        double v0 = vals[a * nl + b];
        double vp = vals[a * nl + b + 1];
        double den = vm - 2. * v0 + vp;
        if (den < -1e-9 && vm > -1e20 && vp > -1e20) {
          db = std::max(-0.5, std::min(0.5, 0.5 * (vm - vp) / den));
        }
      }
      V3 tt = ((a - m) + da) * s * e1 + ((b - m) + db) * s * e2;
      double csc = peaks[i].first / nreal;
      if (cn_ >= 4) {
        double nn_clash = quick_non_neighbor_clash(u, tt);
        if (nn_clash < -0.75 * std::max(csc, 10.0) && nn_clash < -30.) {
          continue;
        }
        csc += 0.3 * nn_clash;
      }
      cands.push_back(Candidate(csc, csc, u, tt));
      ++kept;
    }
  }
  std::stable_sort(cands.begin(), cands.end(),
                   [](const Candidate &a, const Candidate &b) {
                     return a.coarse_score > b.coarse_score;
                   });
  return cands;
}

double CnDocker::score(const Grid3 &g, const V3 &u, const V3 &t,
                       const Ints *fwd_atoms, const Ints *rev_atoms) const {
  Mat3 R = get_rotation_matrix(u, angle_);
  V3 p = get_axis_point(u, t, R);
  double sc = 0.;
  if (cn_ == 2) {
    if (fwd_atoms) {
      for (int i : *fwd_atoms) sc += lookup(g, R * (x_[i] - p) + p);
    } else {
      for (const V3 &x : x_) sc += lookup(g, R * (x - p) + p);
    }
  } else {
    Mat3 Rinv = R.get_transpose();
    double sc_f = 0., sc_r = 0.;
    if (fwd_atoms && rev_atoms) {
      for (int i : *fwd_atoms) sc_f += lookup(g, R * (x_[i] - p) + p);
      for (int i : *rev_atoms) sc_r += lookup(g, Rinv * (x_[i] - p) + p);
    } else {
      for (const V3 &x : x_) {
        V3 dx = x - p;
        sc_f += lookup(g, R * dx + p);
        sc_r += lookup(g, Rinv * dx + p);
      }
    }
    sc = 0.5 * (sc_f + sc_r);
  }
  if (cn_ >= 4) {
    // non-neighbors should not clash (only negative terms count);
    // use a subset of atoms for speed
    for (int k = 2; k <= cn_ / 2; ++k) {
      Mat3 Rk = get_rotation_matrix(u, angle_ * k);
      double neg = 0.;
      for (unsigned i = 0; i < x_.size(); i += ring_stride_) {
        double v = lookup(g, Rk * (x_[i] - p) + p);
        if (v < 0.) neg += v;
      }
      // pairs k and cn-k are equivalent, except when k == cn/2
      sc += neg * ring_stride_ * (2 * k < cn_ ? 2 : 1);
    }
  }
  return sc;
}

void CnDocker::get_interface_atoms(const V3 &u, const V3 &t, double margin,
                                   Ints &fwd_atoms, Ints &rev_atoms,
                                   V3 &if_center) const {
  double cut2 = (pot_.get_cutoff() + margin) * (pot_.get_cutoff() + margin);
  Mat3 R = get_rotation_matrix(u, angle_);
  Mat3 Rinv = R.get_transpose();
  V3 p = get_axis_point(u, t, R);
  fwd_atoms.clear();
  rev_atoms.clear();
  if_center = V3(0., 0., 0.);
  for (unsigned i = 0; i < x_.size(); ++i) {
    V3 dx = x_[i] - p;
    V3 nb = R * dx + p;
    unsigned j = nn_->get_nearest_neighbor(nb);
    if (algebra::get_squared_distance(nb, x_[j]) <= cut2) {
      fwd_atoms.push_back(i);
      if_center += x_[i];
    }
    if (cn_ >= 3) {
      V3 nbr = Rinv * dx + p;
      unsigned jr = nn_->get_nearest_neighbor(nbr);
      if (algebra::get_squared_distance(nbr, x_[jr]) <= cut2) {
        rev_atoms.push_back(i);
      }
    }
  }
  if (!fwd_atoms.empty()) if_center /= fwd_atoms.size();
}

Candidate CnDocker::refine(const Candidate &c) {
  V3 cur_u = c.u, cur_t = c.t;
  double best_sc = score(fine_, cur_u, cur_t);
  V3 best_u = cur_u, best_t = cur_t;

  auto run_pass = [&](const Grid3 &g, double ang_step_rad, double trans_step,
                      unsigned max_ev, double margin) {
    V3 u0 = cur_u, t0 = cur_t;
    V3 e1, e2;
    get_plane_basis(u0, e1, e2);
    Ints fwd_atoms, rev_atoms;
    V3 q;
    get_interface_atoms(u0, t0, margin, fwd_atoms, rev_atoms, q);
    if (fwd_atoms.empty() && rev_atoms.empty()) return;
    Mat3 R0 = get_rotation_matrix(u0, angle_);
    V3 R0q = R0 * q;
    // Parameterize in normalized coordinates with axis tilt pivoted around the
    // interface centroid q and translation stepped directly in the plane
    // perpendicular to u (independent of cn and ring radius)
    auto unpack = [&](const std::vector<double> &v, V3 &uu, V3 &tt) {
      uu = (u0 + (v[0] * ang_step_rad) * e1 + (v[1] * ang_step_rad) * e2)
               .get_unit_vector();
      Mat3 Ru = get_rotation_matrix(uu, angle_);
      V3 t_piv = t0 - (Ru * q - R0q);
      tt = t_piv + (v[2] * trans_step) * e1 + (v[3] * trans_step) * e2;
      tt -= (tt * uu) * uu;
    };
    auto f = [&](const std::vector<double> &v) {
      V3 uu, tt;
      unpack(v, uu, tt);
      return -score(g, uu, tt, &fwd_atoms, &rev_atoms);
    };
    std::vector<std::vector<double> > simplex = {
        {0., 0., 0., 0.}, {1., 0., 0., 0.}, {0., 1., 0., 0.},
        {0., 0., 1., 0.}, {0., 0., 0., 1.}};
    std::vector<double> opt = nelder_mead(f, simplex, 1e-2, 0.25, max_ev);
    unpack(opt, cur_u, cur_t);
  };

  double ang_rad = params_.angle_step * PI / 180.;
  unsigned ev1 = std::max(30u, params_.refine_max_evaluations / 3);
  unsigned ev2 = params_.refine_max_evaluations > ev1
                     ? params_.refine_max_evaluations - ev1
                     : ev1;
  // Pass 1: broad capture on softened potential grid
  run_pass(soft_, 0.65 * ang_rad, 0.75 * params_.coarse_spacing, ev1, 5.5);
  double sc1 = score(fine_, cur_u, cur_t);
  if (sc1 > best_sc) {
    best_sc = sc1;
    best_u = cur_u;
    best_t = cur_t;
  }

  // Pass 2: fine polish on sharp potential grid with refreshed interface atoms
  run_pass(fine_, 0.35 * ang_rad, 0.35 * params_.coarse_spacing, ev2, 4.0);
  double sc2 = score(fine_, cur_u, cur_t);
  if (sc2 > best_sc) {
    best_sc = sc2;
    best_u = cur_u;
    best_t = cur_t;
  }

  return Candidate(best_sc, c.coarse_score, best_u, best_t);
}

std::vector<Candidate> CnDocker::cluster(const std::vector<Candidate> &sols,
                                         double rmsd_thr,
                                         bool keep_nonpos) const {
  // Greedy clustering (sols sorted by decreasing score) by the RMSD of the
  // placed neighbor (T or T^-1 give the same ring)
  unsigned step = std::max(static_cast<size_t>(1), x_.size() / 80);
  algebra::Vector3Ds rep;
  for (unsigned i = 0; i < x_.size(); i += step) rep.push_back(x_[i]);
  double thr = rmsd_thr * rmsd_thr * rep.size();
  std::vector<algebra::Vector3Ds> kept_pos;
  std::vector<Candidate> ret;
  for (const Candidate &c : sols) {
    if (!keep_nonpos && c.score <= 0.) continue;
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
  // rescore on the softened fine grid (tolerant of coarse grid offset)
  if (cands.size() > params_.n_rescore) {
    cands.erase(cands.begin() + params_.n_rescore, cands.end());
  }
  for (Candidate &c : cands) c.score = score(soft_, c.u, c.t);
  std::stable_sort(cands.begin(), cands.end());

  // Pre-cluster before refinement so refinement budget covers distinct basins
  std::vector<Candidate> distinct =
      cluster(cands, std::max(2.0, params_.cluster_rmsd), true);

  std::vector<size_t> refine_indices;
  std::vector<bool> picked(distinct.size(), false);
  unsigned n_soft_pick = (params_.n_refine * 3) / 4;
  for (size_t i = 0; i < distinct.size() && refine_indices.size() < n_soft_pick;
       ++i) {
    picked[i] = true;
    refine_indices.push_back(i);
  }
  // Also refine top distinct clusters by coarse FFT score
  std::vector<size_t> coarse_ord(distinct.size());
  for (size_t i = 0; i < distinct.size(); ++i) coarse_ord[i] = i;
  std::stable_sort(coarse_ord.begin(), coarse_ord.end(),
                   [&](size_t a, size_t b) {
                     return distinct[a].coarse_score > distinct[b].coarse_score;
                   });
  for (size_t idx : coarse_ord) {
    if (refine_indices.size() >= params_.n_refine) break;
    if (picked[idx]) continue;
    picked[idx] = true;
    refine_indices.push_back(idx);
  }
  for (size_t i = 0;
       i < distinct.size() && refine_indices.size() < params_.n_refine; ++i) {
    if (picked[i]) continue;
    picked[i] = true;
    refine_indices.push_back(i);
  }

  for (size_t idx : refine_indices) {
    distinct[idx] = refine(distinct[idx]);
  }
  // Bring the next top unrefined candidates onto the same fine-grid score scale
  unsigned n_unref = 0;
  for (size_t i = 0; i < distinct.size() && n_unref < 300; ++i) {
    if (!picked[i]) {
      distinct[i].score = score(fine_, distinct[i].u, distinct[i].t);
      ++n_unref;
    }
  }
  std::stable_sort(distinct.begin(), distinct.end());
  std::vector<Candidate> ret = cluster(distinct, params_.cluster_rmsd, false);
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
