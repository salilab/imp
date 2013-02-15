/**
 *  \file connolly_surface.cpp     \brief Generate surface for a set of atoms
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

/* Note that more complete documentation of the original Connolly method
   can be found in QCPE, program 429, e.g. at
   http://atlas.physbio.mssm.edu/~mezei/molmod_core/docs/connolly.html
 */

#include <IMP/multifit/connolly_surface.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/constants.h>
#include <IMP/atom/pdb.h>
#include <boost/multi_array.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

IMPMULTIFIT_BEGIN_NAMESPACE

/* Put GridPoint in a non-anonymous namespace, since g++ 4.2 has a bug which
   results in a compilation failure if it's in the anonymous namespace:
   https://svn.boost.org/trac/boost/ticket/3729
 */
namespace detail {
  struct GridPoint {
    GridPoint() : icube(-1), scube(false), sscube(false) {}

    int icube;
    bool scube, sscube;
  };
}

namespace {

static const int MAXSPH=1000;
static const int MAXCIR=1000;

struct AtomTypeInfo {
  // unit vectors
  algebra::Vector3Ds ua;

  // extended vectors
  algebra::Vector3Ds eva;
};

struct AtomInfo {
  AtomInfo() : ico(3), skip(false), icuptr(-1) {}

  // Integer cube coordinate
  std::vector<int> ico;

  // Skip from grid
  bool skip;

  // Index of next atom in same cube as this one
  int icuptr;
};

class Cube {
public:
  void grid_coordinates(const algebra::Vector3Ds &CO, float radmax, float rp) {
    // calculate width of cube from maximum atom radius and probe radius
    width_ = 2. * (radmax + rp);

    // minimum atomic coordinates (cube corner)
    algebra::Vector3D comin(1000000.0, 1000000.0, 1000000.0);
    for (unsigned n = 0; n < CO.size(); ++n) {
      for (int k = 0; k < 3; ++k) {
        comin[k] = std::min(comin[k], CO[n][k]);
      }
    }

    dim_ = 0;
    atom_info_.resize(CO.size());
    for (unsigned n = 0; n < CO.size(); ++n) {
      for (int k = 0; k < 3; ++k) {
        int ico = static_cast<int>((CO[n][k] - comin[k]) / width_);
        IMP_INTERNAL_CHECK(ico >= 0, "cube coordinate out of range");
        dim_ = std::max(dim_, ico);
        atom_info_[n].ico[k] = ico;
      }
    }
    dim_++;

    cube_.resize(boost::extents[dim_][dim_][dim_]);

    for (unsigned n = 0; n < CO.size(); ++n) {
      AtomInfo &ai = atom_info_[n];
      if (!ai.skip) {
        add_atom_to_cube(CO, n);
      }
    }

    update_adjacent();
  }

  bool get_neighbors(int n, const algebra::Vector3Ds &CO,
                     float rp, const std::vector<int> &IAT,
                     const std::vector<float> &rtype,
                     std::vector<int> &neighbors) {
    AtomInfo &ai = atom_info_[n];
    neighbors.resize(0);
    if (ai.skip) {
      return false;
    }
    int ici = ai.ico[0];
    int icj = ai.ico[1];
    int ick = ai.ico[2];
    if (!cube_[ici][icj][ick].sscube) {
      return false;
    }
    float sumi = 2 * rp + rtype[IAT[n]];
    for (int jck = ick - 1; jck <= ick + 1; ++jck) {
      if (jck >= 0 && jck < dim_) {
        for (int jcj = icj - 1; jcj <= icj + 1; ++jcj) {
          if (jcj >= 0 && jcj < dim_) {
            for (int jci = ici - 1; jci <= ici + 1; ++jci) {
              if (jci >= 0 && jci < dim_) {
                for (int jatom = cube_[jci][jcj][jck].icube;
                     jatom >= 0; jatom = atom_info_[jatom].icuptr) {
                  float sum = sumi + rtype[IAT[jatom]];
                  if (n != jatom &&
                      algebra::get_squared_distance(CO[n], CO[jatom])
                                         < sum*sum) {
                    neighbors.push_back(jatom);
                  }
                }
              }
            }
          }
        }
      }
    }
    return true;
  }

private:
  void add_atom_to_cube(const algebra::Vector3Ds &CO, int n) {
    AtomInfo &ai = atom_info_[n];
    int existing_atom = cube_[ai.ico[0]][ai.ico[1]][ai.ico[2]].icube;
    if (existing_atom < 0) {
      // first atom in this cube
      cube_[ai.ico[0]][ai.ico[1]][ai.ico[2]].icube = n;
      cube_[ai.ico[0]][ai.ico[1]][ai.ico[2]].scube = true;
    } else {
      while(true) {
        double dist = algebra::get_squared_distance(CO[n], CO[existing_atom]);
        if (dist <= 0.) {
          ai.skip = true;
          IMP_LOG_VERBOSE( "Skipped atom " << n << " with same coordinates as "
                           << existing_atom << std::endl);
          return;
        }
        if (atom_info_[existing_atom].icuptr == -1) {
          atom_info_[existing_atom].icuptr = n;
          return;
        }
        existing_atom = atom_info_[existing_atom].icuptr;
      }
    }
  }

  void update_adjacent() {
    for (int i = 0; i < dim_; ++i) {
      for (int j = 0; j < dim_; ++j) {
        for (int k = 0; k < dim_; ++k) {
          update_cube(i, j, k);
        }
      }
    }
  }

  void update_cube(int i, int j, int k) {
    if (cube_[i][j][k].icube >= 0) {
      for (int i1 = i - 1; i1 <= i + 1; ++i1) if (i1 >=0 && i1 < dim_) {
        for (int j1 = j - 1; j1 <= j + 1; ++j1) if (j1 >=0 && j1 < dim_) {
          for (int k1 = k - 1; k1 <= k + 1; ++k1) {
            if (k1 >=0 && k1 < dim_ && cube_[i1][j1][k1].scube) {
              cube_[i][j][k].sscube = true;
              return;
            }
          }
        }
      }
    }
  }

  int dim_;
  float width_;
  std::vector<AtomInfo> atom_info_;
  boost::multi_array<detail::GridPoint, 3> cube_;
};

struct YonProbe {
  YonProbe(algebra::Vector3D py, algebra::Vector3D ay)
          : center(py), altitude(ay) {}
  algebra::Vector3D center;
  algebra::Vector3D altitude;
};

struct SurfacePoint {
  bool yon;
  algebra::Vector3D s;
  float area;
  int n1, n2, n3;
};

enum PointType { VICTIM, YON, OTHER };

struct ProbePoint {
  ProbePoint() : type(OTHER) {}

  char ishape;
  algebra::Vector3D position;
  algebra::Vector3D to_center;
  PointType type;
  std::vector<SurfacePoint> points;
};

class YonCube {
public:
  YonCube(const std::vector<YonProbe> &yon_probes, float rp, float dp,
          float radmax)
          : comin_(1000000.0, 1000000.0, 1000000.0) {
    width_ = 2. * (radmax + rp);
    dp2_ = dp * dp;
    for (unsigned n = 0; n < yon_probes.size(); ++n) {
      for (int k = 0; k < 3; ++k) {
        comin_[k] = std::min(comin_[k], yon_probes[n].center[k]);
      }
    }

    dim_ = 0;
    atom_info_.resize(yon_probes.size());
    for (unsigned n = 0; n < yon_probes.size(); ++n) {
      atom_info_[n].ico = get_cube_coordinates(yon_probes[n].center);
      for (int k = 0; k < 3; ++k) {
        dim_ = std::max(dim_, atom_info_[n].ico[k]);
      }
    }
    dim_++;
    cube_.resize(boost::extents[dim_][dim_][dim_]);
    for (int i = 0; i < dim_; ++i) {
      for (int j = 0; j < dim_; ++j) {
        for (int k = 0; k < dim_; ++k) {
          cube_[i][j][k] = -1;
        }
      }
    }
    for (unsigned n = 0; n < yon_probes.size(); ++n) {
      add_probe_to_cube(n);
    }
  }

  bool probe_overlap(const ProbePoint &probe,
                     const std::vector<YonProbe> &yon_probes) {
    std::vector<int> ic = get_cube_coordinates(probe.position);
    for (int k = 0; k < 3; ++k) {
      ic[k] = std::max(ic[k], 0);
      ic[k] = std::min(ic[k], dim_ - 1);
    }
    // Check adjoining cubes
    for (int jci = ic[0] - 1; jci <= ic[0] + 1; ++jci) {
      if (jci >= 0 && jci < dim_) {
        for (int jcj = ic[1] - 1; jcj <= ic[1] + 1; ++jcj) {
          if (jcj >= 0 && jcj < dim_) {
            for (int jck = ic[2] - 1; jck <= ic[2] + 1; ++jck) {
              if (jck >= 0 && jck < dim_) {
                for (int jp = cube_[jci][jcj][jck];
                     jp >= 0; jp = atom_info_[jp].icuptr) {
                  const YonProbe &yp = yon_probes[jp];
                  if (algebra::get_squared_distance(yp.center,
                                                    probe.position) < dp2_
                      && yp.altitude * probe.to_center < 0) {
                    return true;
                  }
                }
              }
            }
          }
        }
      }
    }
    return false;
  }

private:
  void add_probe_to_cube(int n) {
    AtomInfo &ai = atom_info_[n];
    int existing_atom = cube_[ai.ico[0]][ai.ico[1]][ai.ico[2]];
    if (existing_atom < 0) {
      cube_[ai.ico[0]][ai.ico[1]][ai.ico[2]] = n;
    } else {
      while(atom_info_[existing_atom].icuptr != -1) {
        existing_atom = atom_info_[existing_atom].icuptr;
      }
      atom_info_[existing_atom].icuptr = n;
    }
  }

  std::vector<int> get_cube_coordinates(const algebra::Vector3D &c) {
    std::vector<int> ret;
    for (int k = 0; k < 3; ++k) {
      ret.push_back(static_cast<int>((c[k] - comin_[k]) / width_));
    }
    return ret;
  }

  algebra::Vector3D comin_;
  int dim_;
  float width_;
  float dp2_;
  std::vector<AtomInfo> atom_info_;
  boost::multi_array<int, 3> cube_;
};

// Generate unit vectors over sphere
void genun(algebra::Vector3Ds &vec, unsigned n)
{
  vec.reserve(n);
  vec.resize(0);

  int nequat = static_cast<int>(std::sqrt(n * 3.14159));
  int nvert = std::max(1, nequat / 2);

  for (int i = 0; i <= nvert; ++i) {
    float fi = (3.14159 * i) / nvert;
    float z = std::cos(fi);
    float xy = std::sin(fi);
    int nhor = std::max(1, static_cast<int>(nequat * xy));
    for (int j = 0; j < nhor; ++j) {
      float fj = (2. * 3.14159 * j) / nhor;
      float x = std::cos(fj) * xy;
      float y = std::sin(fj) * xy;
      if (vec.size() >= n) return;
      vec.push_back(algebra::Vector3D(x, y, z));
    }
  }
}

// Triple product of three vectors
float det(const algebra::Vector3D &a,
          const algebra::Vector3D &b,
          const algebra::Vector3D &c)
{
  algebra::Vector3D ab = algebra::get_vector_product(a, b);
  return ab * c;
}

// Make identity matrix
void imatx(algebra::Vector3Ds &ghgt)
{
  ghgt.resize(3);
  ghgt[0] = algebra::Vector3D(1., 0., 0.);
  ghgt[1] = algebra::Vector3D(0., 1., 0.);
  ghgt[2] = algebra::Vector3D(0., 0., 1.);
}

// Concatenate matrix b into matrix a
void cat(algebra::Vector3Ds &a, const algebra::Vector3Ds &b)
{
  algebra::Vector3Ds temp(3);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      temp[j][i] = a[0][i]*b[j][0] + a[1][i]*b[j][1] + a[2][i]*b[j][2];
    }
  }
  a = temp;
}

// Conjugate matrix g with matrix h giving ghgt
void conj(const algebra::Vector3Ds &h,
          const algebra::Vector3Ds &g,
          algebra::Vector3Ds &ghgt)
{
  // INITIALIZE GHGT MATRIX TO IDENTITY
  imatx(ghgt);
  // CONCATENATE G H GT
  cat(ghgt,g);
  cat(ghgt,h);
  // CALCULATE GT
  algebra::Vector3Ds gt(3);
  for (int k = 0; k < 3; ++k) {
    for (int l = 0; l < 3; ++l) {
      gt[l][k] = g[k][l];
    }
  }
  cat(ghgt,gt);
}

// Multiply v by a giving w
void multv(const algebra::Vector3D &v, const algebra::Vector3Ds &a,
           algebra::Vector3D &w)
{
  for (int i = 0; i < 3; ++i) {
    w[i] = a[0][i] * v[0] + a[1][i] * v[1] + a[2][i] * v[2];
  }
}

// Return b perpendicular to a
algebra::Vector3D vperp(const algebra::Vector3D &a)
{
  algebra::Vector3D b(0.,0.,0.), p;

  // FIND SMALLEST COMPONENT
  float small = 10000.0;
  int m = -1;
  for (int k=0; k < 3; ++k) {
    if (std::abs(a[k]) < small) {
      small = std::abs(a[k]);
      m = k;
    }
  }
  b[m] = 1.0;

  // take projection along a and subtract from b
  float dt = a[m] / a.get_squared_magnitude();
  b = b - dt * a;
  // renormalize b
  return b.get_unit_vector();
}

// Collision check of probe with neighboring atoms
bool collid(const algebra::Vector3D &p, const algebra::Vector3Ds &cnbr,
            const std::vector<float> &ernbr, int jnbr, int knbr, int lkf,
            const std::vector<int> &lknbr)
{
  for (int i = lkf; i >= 0; i = lknbr[i]) {
    if (i == jnbr || i == knbr) continue;
    float dist2 = algebra::get_squared_distance(p, cnbr[i]);
    if (dist2 < ernbr[i] * ernbr[i]) {
      return true;
    }
  }
  return false;
}

struct SurfaceInfo {
  SurfaceInfo() : area(0.), npoints(0), nlost_saddle(0), nlost_concave(0) {}

  float area;
  int npoints;
  int nlost_saddle;
  int nlost_concave;
};

void handle_atom(std::ostream &fout,
                 int iatom, float d, const std::vector<int> inbr,
                 const algebra::Vector3Ds &CO, float rp,
                 std::vector<bool> &srs,
                 const algebra::Vector3Ds &up,
                 std::vector<YonProbe> &yon_probes,
                 const algebra::Vector3Ds &circle,
                 const std::vector<int> &IAT,
                 const std::vector<float> &rtype,
                 const std::vector<AtomTypeInfo> &attyp_info,
                 std::vector<ProbePoint> &beforept,
                 SurfaceInfo &surface)
{
  float ri = rtype[IAT[iatom]];
  algebra::Vector3D ci = CO[iatom];

  /* transfer data from main arrays to neighbors */
  algebra::Vector3Ds cnbr;
  std::vector<float> rnbr;
  std::vector<float> ernbr;
  std::vector<float> disnbr;
  std::vector<int> lknbr;
  for (unsigned iuse = 0; iuse < inbr.size(); ++iuse) {
    int jatom = inbr[iuse];
    cnbr.push_back(CO[jatom]);
    rnbr.push_back(rtype[IAT[jatom]]);
    ernbr.push_back(rtype[IAT[jatom]] + rp);
    disnbr.push_back(algebra::get_squared_distance(ci, CO[jatom]));
    // initialize link to next farthest out neighbor
    lknbr.push_back(-1);
  }

  // Set up a linked list of neighbors in order of
  // increasing distance from iatom
  int lkf = 0;
  // put remaining neighbors in linked list at proper position
  for (int l = lkf+1; l < static_cast<int>(inbr.size()); ++l) {
    int before = -1;
    int after = lkf;
    // step through the list until before:after bracket the current neighbor
    while (after >= 0 && disnbr[l] < disnbr[after]) {
      before = after;
      after = lknbr[after];
    }
    if (before < 0) {
      lkf = l;
    } else {
      lknbr[before] = l;
    }
    lknbr[l] = after;
  }
  // Handle no-neighbors case
  if (inbr.size() == 0) {
    lkf = -1;
  }

  // medium loop for each neighbor of iatom
  for (unsigned jnbr = 0; jnbr < inbr.size(); ++jnbr) {
    int jatom = inbr[jnbr];
    if (jatom <= iatom) continue;

    float rj = rnbr[jnbr];
    algebra::Vector3D cj = cnbr[jnbr];

    /* HERE FOLLOW GEOMETRIC CALCULATIONS OF POINTS, VECTORS AND
       DISTANCES USED FOR PROBE PLACEMENT IN BOTH SADDLE AND
       CONCAVE REENTRANT SURFACE GENERATION */

    /* CALCULATE THE INTERSECTION
       OF THE EXPANDED SPHERES OF IATOM AND JATOM
       THIS CIRCLE IS CALLED THE SADDLE CIRCLE
       THE PLANE IT LIES IN IS CALLED THE SADDLE PLANE */

    algebra::Vector3D vij = cj - ci;

    /* CREATE AN ORTHONORMAL FRAME
       WITH UIJ POINTING ALONG THE INTER-ATOMIC AXIS
       AND Q AND T DEFINING THE SADDLE PLANE */
    float dij = vij.get_magnitude();
    if (dij <= 0.) {
      IMP_LOG_VERBOSE( "Atoms " << iatom << " and " << jatom
                       << " have the same center" << std::endl);
      continue;
    }
    algebra::Vector3D uij = vij.get_unit_vector();
    algebra::Vector3D q = vperp(uij);
    algebra::Vector3D t = algebra::get_vector_product(uij, q);

    // CALCULATE THE SADDLE CIRCLE CENTER AND RADIUS
    float f = 0.5*(1.0+((ri+rp)*(ri+rp)-(rj+rp)*(rj+rp))/(dij*dij));
    // BASE POINT
    algebra::Vector3D bij = ci + f * vij;
    float f1 = ri+rj+2.*rp;
    f1 = f1 * f1 - dij * dij;
    // SKIP TO BOTTOM OF MIDDLE LOOP IF ATOMS ARE TOO FAR APART
    if (f1 <= 0.0) continue;

    float f2 = dij*dij - (ri-rj)*(ri-rj);
    // SKIP TO BOTTOM OF MIDDLE LOOP IF ONE ATOM INSIDE THE OTHER
    if (f2 <= 0.0) continue;

    // HEIGHT (RADIUS OF SADDLE CIRCLE)
    float hij = std::sqrt(f1*f2) / (2.*dij);
    // A STARTING ALTITUDE
    algebra::Vector3D aij = hij * q;

    // CONCAVE REENTRANT SURFACE

    // GATHER MUTUAL NEIGHBORS OF IATOM AND JATOM
    int mutual = 0;
    std::vector<bool> mnbr(inbr.size());
    for (unsigned knbr = 0; knbr < inbr.size(); ++knbr) {
      float d2 = algebra::get_squared_distance(cj, cnbr[knbr]);
      float radsum = 2. * rp + rj + rnbr[knbr];
      mnbr[knbr] = d2 < radsum*radsum && knbr != jnbr;
      if (mnbr[knbr]) {
        ++mutual;
      }
    }

    // INNER LOOP FOR EACH MUTUAL NEIGHBOR OF IATOM AND JATOM
    char ishape = 3;
    for (unsigned knbr = 0; knbr < inbr.size(); ++knbr) {
      if (!mnbr[knbr]) continue;

      int katom = inbr[knbr];
      if (katom <= jatom) continue;

      // transfer from neighbor array to katom variables
      float rk = rnbr[knbr];
      algebra::Vector3D ck = cnbr[knbr];

      /* CALCULATE INTERSECTION OF EXPANDED SPHERE OF KATOM
         WITH SADDLE PLANE. WE WILL CALL THIS THE KATOM CIRCLE. */

      /* PROJECTION OF VECTOR,
         FROM KATOM TO A POINT ON THE SADDLE PLANE,
         ONTO IATOM-JATOM AXIS,
         IN ORDER TO GET DISTANCE KATOM IS FROM SADDLE PLANE */
      float dk = uij[0] * (bij[0]-ck[0]) + uij[1] * (bij[1]-ck[1])
                 + uij[2] * (bij[2]-ck[2]);

      // CALCULATE RADIUS OF KATOM CIRCLE
      float rijk = (rk+rp) * (rk+rp) - dk * dk;
      // SKIP CONCAVE CALCULATION IF NO INTERSECTION
      if (rijk <= 0.0) continue;
      rijk = std::sqrt(rijk);

      // CALCULATE CENTER OF KATOM CIRCLE
      algebra::Vector3D cijk = ck + dk * uij;

      // CALCULATE INTERSECTION OF THE KATOM CIRCLE WITH THE SADDLE CIRCLE
      algebra::Vector3D vijk = cijk - bij;
      float dijk = vijk.get_magnitude();

      if (dijk <= 0.0) {
        IMP_LOG_VERBOSE( "Atoms " << iatom << ", " << jatom << ", and "
                       << katom << " have concentric circles" << std::endl);
        continue;
      }
      float f = 0.5 * (1.0+(hij*hij - rijk*rijk)/(dijk*dijk));
      // BASE POINT BIJK IS ON SYMMETRY PLANE AND SADDLE PLANE
      algebra::Vector3D bijk = bij + f * vijk;

      float f1 = (hij+rijk)*(hij+rijk) - dijk*dijk;
      // SKIP TO BOTTOM OF INNER LOOP IF KATOM TOO FAR AWAY
      if (f1 <= 0.0) continue;

      float f2 = dijk*dijk - (hij-rijk)*(hij-rijk);
      // SKIP TO BOTTOM OF INNER LOOP IF KATOM CIRCLE INSIDE SADDLE CIRCLE
      // OR VICE-VERSA
      if (f2 <= 0.0) continue;

      float hijk = std::sqrt(f1*f2) / (2.*dijk);
      algebra::Vector3D uijk = vijk.get_unit_vector();

      // UIJ AND UIJK LIE IN THE SYMMETRY PLANE PASSING THROUGH THE ATOMS
      // SO THEIR CROSS PRODUCT IS PERPENDICULAR TO THIS PLANE
      algebra::Vector3D aijk0 = algebra::get_vector_product(uij, uijk);

      algebra::Vector3Ds aijk;
      aijk.push_back(aijk0 * hijk);
      aijk.push_back(-aijk0 * hijk);

      // PROBE PLACEMENT AT ENDS OF ALTITUDE VECTORS
      algebra::Vector3Ds pijk(2);
      std::vector<bool> pair(2);
      for (int ip = 0; ip < 2; ++ip) {
        pijk[ip] = bijk + aijk[ip];
        // COLLISION CHECK WITH MUTUAL NEIGHBORS
        pair[ip] = !collid(pijk[ip],cnbr,ernbr,jnbr,knbr,lkf,lknbr);
      }
      // IF NEITHER PROBE POSITION IS ALLOWED, SKIP TO BOTTOM OF INNER LOOP
      if (!pair[0] && !pair[1]) continue;
      bool both = pair[0] && pair[1];
      // SOME REENTRANT SURFACE FOR ALL THREE ATOMS
      srs[iatom] = srs[jatom] = srs[katom] = true;

      // GENERATE SURFACE POINTS
      float area = (4. * algebra::PI * rp * rp) / up.size();
      for (int ip = 0; ip < 2; ++ip) {
        if (!pair[ip]) continue;

        // DETERMINE WHETHER PROBE HAS SURFACE ON FAR SIDE OF PLANE
        bool yonprb = hijk < rp && !both;
        // CALCULATE VECTORS DEFINING SPHERICAL TRIANGLE
        // THE VECTORS ARE GIVEN THE PROBE RADIUS AS A LENGTH
        // ONLY FOR THE PURPOSE OF MAKING THE GEOMETRY MORE CLEAR
        algebra::Vector3D vpi = (ci - pijk[ip]) * rp / (ri + rp);
        algebra::Vector3D vpj = (cj - pijk[ip]) * rp / (rj + rp);
        algebra::Vector3D vpk = (ck - pijk[ip]) * rp / (rk + rp);
        float sign = det(vpi,vpj,vpk);

        ProbePoint probe_point;
        // GATHER POINTS ON PROBE SPHERE LYING WITHIN TRIANGLE
        for (unsigned i = 0; i < up.size(); ++i) {
          SurfacePoint sp;
          // IF THE UNIT VECTOR IS POINTING AWAY FROM THE SYMMETRY PLANE
          // THE SURFACE POINT CANNOT LIE WITHIN THE INWARD-FACING TRIANGLE
          if (up[i] * aijk[ip] > 0.) continue;
          if (sign * det(up[i],vpj,vpk) < 0.) continue;
          if (sign * det(vpi,up[i],vpk) < 0.) continue;
          if (sign * det(vpi,vpj,up[i]) < 0.) continue;
          // CALCULATED WHETHER POINT IS ON YON SIDE OF PLANE
          sp.yon = aijk[ip] * (aijk[ip] + up[i]) < 0.;
          // OVERLAPPING REENTRANT SURFACE REMOVAL
          // FOR SYMMETRY-RELATED PROBE POSITIONS
          if (sp.yon && both) continue;

          // CALCULATE COORDINATES OF SURFACE POINT
          sp.s = pijk[ip] + up[i] * rp;

          // FIND THE CLOSEST ATOM AND PUT THE THREE ATOM NUMBERS
          // IN THE PROPER ORDER
          // N1 IS CLOSEST, N2 < N3
          float dsi = algebra::get_distance(sp.s, ci) - ri;
          float dsj = algebra::get_distance(sp.s, cj) - rj;
          float dsk = algebra::get_distance(sp.s, ck) - rk;
          if (dsi <= dsj && dsi <= dsk) {
            sp.n1 = iatom;
            sp.n2 = jatom;
            sp.n3 = katom;
          } else if (dsj <= dsi && dsj <= dsk) {
            sp.n1 = jatom;
            sp.n2 = iatom;
            sp.n3 = katom;
          } else {
            sp.n1 = katom;
            sp.n2 = iatom;
            sp.n3 = jatom;
          }
          sp.area = area;
          probe_point.points.push_back(sp);
        }
        if (probe_point.points.size() > 0) {
          probe_point.ishape = ishape;
          probe_point.position = pijk[ip];
          probe_point.to_center = aijk[ip];
          probe_point.type = yonprb ? YON : OTHER;
          beforept.push_back(probe_point);

          // SAVE PROBE IN YON PROBE ARRAYS
          if (yonprb) {
            yon_probes.push_back(YonProbe(pijk[ip], aijk[ip]));
          }
        }
      }
    }

    // SADDLE-SHAPED REENTRANT
    ishape = 2;

    // SPECIAL CHECK FOR BURIED TORI

    /* IF NEITHER ATOM HAS ANY REENTRANT SURFACE SO FAR
       (AFTER TRIANGLES WITH ALL KATOMS HAVE BEEN CHECKED)
       AND IF THERE IS SOME MUTUAL NEIGHBOR IN THE SAME MOLECULE
       CLOSE ENOUGH SO THAT THE TORUS CANNOT BE FREE,
       THEN WE KNOW THAT THIS MUST BE A BURIED TORUS */
    if (!srs[iatom] && !srs[jatom] && mutual > 0) {
      bool buried_torus = false;
      for (unsigned knbr = 0; knbr < inbr.size() && !buried_torus; ++knbr) {
        if (!mnbr[knbr]) continue;
        float d2 = algebra::get_squared_distance(bij, cnbr[knbr]);
        float rk2 = ernbr[knbr]*ernbr[knbr] - hij*hij;
        if (d2 < rk2) {
          buried_torus = true;
        }
      }
      if (buried_torus) continue;
    }

    // CALCULATE NUMBER OF ROTATIONS OF PROBE PAIR,
    // ROTATION ANGLE AND ROTATION MATRIX
    float rij = ri/(ri + rp) + rj/(rj + rp);
    float avh = (std::abs(hij-rp) + hij * rij)/3.;
    int nrot = std::max(static_cast<int>(std::sqrt(d) * algebra::PI * avh), 1);
    float angle = algebra::PI / nrot;

    // SET UP ROTATION MATRIX AROUND X-AXIS
    algebra::Vector3Ds h(3, algebra::Vector3D(0., 0., 0.));
    h[0][0] = 1.;
    h[1][1] = h[2][2] = std::cos(angle);
    h[1][2] = std::sin(angle);
    h[2][1] = -h[1][2];
    // CALCULATE MATRIX TO ROTATE X-AXIS ONTO IATOM-JATOM AXIS
    algebra::Vector3Ds g;
    g.push_back(uij);
    g.push_back(q);
    g.push_back(t);

    // MAKE THE PROBE PAIR ROTATION MATRIX BE ABOUT THE IATOM-JATOM AXIS
    algebra::Vector3Ds ghgt;
    conj(h, g, ghgt);

    // ARC GENERATION
    algebra::Vector3D pij = bij + aij;
    algebra::Vector3D vpi = (ci - pij) * rp / (ri + rp);
    algebra::Vector3D vpj = (cj - pij) * rp / (rj + rp);

    // ROTATE CIRCLE ONTO IATOM-JATOM-PROBE PLANE
    // AND SELECT POINTS BETWEEN PROBE-IATOM AND
    // PROBE-JATOM VECTOR TO FORM THE ARC
    int narc = 0;
    std::vector<bool> ayon;
    std::vector<float> arca;
    algebra::Vector3Ds vbs0[2];
    for (unsigned i = 0; i < circle.size(); ++i) {
      algebra::Vector3D vps0;
      // ROTATION
      multv(circle[i], g, vps0);
      // IF THE VECTOR IS POINTING AWAY FROM THE SYMMETRY LINE
      // THE SURFACE POINT CANNOT LIE ON THE INWARD-FACING ARC
      if (vps0 * aij > 0.) continue;
      algebra::Vector3D vector = algebra::get_vector_product(vpi, vps0);
      if (g[2] * vector < 0.) continue;
      vector = algebra::get_vector_product(vps0, vpj);
      if (g[2] * vector < 0.) continue;

      /* MAKE ARC POINT VECTORS ORIGINATE WITH SADDLE CIRCLE CENTER BIJ
         RATHER THAN PROBE CENTER BECAUSE THEY WILL BE
         ROTATED AROUND THE IATOM-JATOM AXIS */
      vbs0[0].push_back(vps0 + aij);
      // INVERT ARC THROUGH LINE OF SYMMETRY
      float duij = uij * vbs0[0][narc];
      vbs0[1].push_back(-vbs0[0][narc] + 2 * duij * uij);

      /* CHECK WHETHER THE ARC POINT CROSSES THE IATOM-JATOM AXIS
         AND CALCULATE THE AREA ASSOCIATED WITH THE POINT */
      float ht = aij * vbs0[0][narc]/hij;
      ayon.push_back(ht < 0.);
      arca.push_back((2.*algebra::PI*algebra::PI * rp * std::abs(ht))
                     /(circle.size()*nrot));
      narc++;
    }

    // INITIALIZE POWER MATRIX TO IDENTITY
    algebra::Vector3Ds pow;
    imatx(pow);

    // ROTATE THE PROBE PAIR AROUND THE PAIR OF ATOMS
    for (int irot = 0; irot < nrot; ++irot, cat(pow,ghgt)) {
      algebra::Vector3Ds aijp(2);
      // MULTIPLY ALTITUDE VECTOR BY POWER MATRIX
      multv(aij, pow, aijp[0]);
      // SET UP OPPOSING ALTITUDE
      aijp[1] = -aijp[0];

      // SET UP PROBE SPHERE POSITIONS
      algebra::Vector3Ds pijp(2);
      bool pair[2];
      for (int ip = 0; ip < 2; ++ip) {
        pijp[ip] = bij + aijp[ip];
        // CHECK FOR COLLISIONS WITH NEIGHBORING ATOMS
        pair[ip] = !collid(pijp[ip],cnbr,ernbr,jnbr,-1,lkf,lknbr);
      }

      // NO SURFACE GENERATION IF NEITHER PROBE POSITION IS ALLOWED
      if (!pair[0] && !pair[1]) continue;
      bool both = pair[0] && pair[1];
      // SOME REENTRANT SURFACE FOR BOTH ATOMS
      srs[iatom] = srs[jatom] = true;

      /* SKIP TO BOTTOM OF MIDDLE LOOP IF IATOM AND JATOM
         ARE CLOSE ENOUGH AND THE SURFACE POINT DENSITY IS
         LOW ENOUGH SO THAT THE ARC HAS NO POINTS */
      if (narc <= 0) continue;

      // SURFACE GENERATION
      for (int ip = 0; ip < 2; ++ip) {
        if (!pair[ip]) continue;

        // DETERMINE WHETHER PROBE HAS SURFACE ON FAR SIDE OF LINE
        bool yonprb = hij < rp && !both;
        ProbePoint probe_point;
        // THE SADDLE-SHAPED REENTRANT SURFACE POINTS COME FROM THE ARC
        for (int i = 0; i < narc; ++i) {
          SurfacePoint sp;
          /* OVERLAPPING REENTRANT SURFACE REMOVAL
             FOR SYMMETRY-RELATED PROBE POSITIONS */
          if (both && ayon[i]) continue;
          // ROTATE THE ARC FROM THE XY PLANE ONTO THE IATOM-JATOM-PROBE PLANE
          algebra::Vector3D vbs;
          multv(vbs0[ip][i], pow, vbs);
          // MAKE COORDINATES RELATIVE TO ORIGIN
          sp.s = bij + vbs;
          // FIND THE CLOSEST ATOM AND SET UP THE ATOM NUMBERS FOR THE POINT
          float dsi = algebra::get_distance(sp.s, ci) - ri;
          float dsj = algebra::get_distance(sp.s, cj) - rj;
          if (dsi <= dsj) {
            sp.n1 = iatom;
            sp.n2 = jatom;
          } else {
            sp.n1 = jatom;
            sp.n2 = iatom;
          }
          sp.n3 = -1;

          // WE'VE GOT A SURFACE POINT
          sp.yon = ayon[i];
          sp.area = arca[i];
          probe_point.points.push_back(sp);
        }
        if (probe_point.points.size() > 0) {
          probe_point.ishape = ishape;
          probe_point.position = pijp[ip];
          probe_point.to_center = aijp[ip];
          probe_point.type = yonprb ? YON : OTHER;
          beforept.push_back(probe_point);
          // SAVE PROBE IN YON PROBE ARRAYS
          if (yonprb) {
            yon_probes.push_back(YonProbe(pijp[ip], aijp[ip]));
          }
        }
      }
    }
  }

  /* IF THE PROBE RADIUS IS GREATER THAN ZERO
     AND IATOM HAS AT LEAST ONE NEIGHBOR, BUT NO REENTRANT SURFACE,
     THEN IATOM MUST BE COMPLETELY INACCESSIBLE TO THE PROBE */
  if (rp > 0. && inbr.size() > 0 && !srs[iatom]) return;

  const AtomTypeInfo &attyp = attyp_info[IAT[iatom]];
  float area = (4. * algebra::PI * ri*ri) / attyp.ua.size();

  // CONTACT PROBE PLACEMENT LOOP
  for (unsigned i = 0; i < attyp.ua.size(); ++i) {
    // SET UP PROBE COORDINATES
    algebra::Vector3D pipt = ci + attyp.eva[i];
    // CHECK FOR COLLISION WITH NEIGHBORING ATOMS
    if (collid(pipt,cnbr,ernbr,-1,-1,lkf,lknbr)) continue;

    // INCREMENT SURFACE POINT COUNTER FOR CONVEX SURFACE
    surface.npoints++;

    // ADD SURFACE POINT AREA TO CONTACT AREA
    surface.area += area;
    algebra::Vector3D outco = ci + ri * attyp.ua[i];
    algebra::Vector3D outvec = attyp.ua[i];

    // CONTACT
    fout << std::setw(5) << iatom+1
         << "    0    0"
         << std::fixed
         << std::setw(8) << std::setprecision(3) << outco[0]
         << std::setw(8) << std::setprecision(3) << outco[1]
         << std::setw(8) << std::setprecision(3) << outco[2]
         << std::setw(8) << std::setprecision(3) << area
         << std::setw(7) << std::setprecision(3) << outvec[0]
         << std::setw(7) << std::setprecision(3) << outvec[1]
         << std::setw(7) << std::setprecision(3) << outvec[2]
         << "  0.500" << std::endl;
  }
  return;
}

SurfaceInfo generate_contact_surface(std::ostream &fout,
                              const algebra::Vector3Ds &CO, float radmax,
                              float rp, float d,
                              std::vector<YonProbe> &yon_probes,
                              const std::vector<int> &IAT,
                              const std::vector<float> &rtype,
                              const std::vector<AtomTypeInfo> &attyp_info,
                              std::vector<ProbePoint> &beforept)
{
  Cube cube;
  cube.grid_coordinates(CO, radmax, rp);

  // set up probe sphere and circle
  algebra::Vector3Ds up;
  int nup = static_cast<int>(4. * algebra::PI * rp * rp * d);
  nup = std::max(1, nup);
  nup = std::min(MAXSPH, nup);
  genun(up, nup);

  int ncirc = static_cast<int>(2. * algebra::PI * rp * std::sqrt(d));
  ncirc = std::max(1, ncirc);
  ncirc = std::min(MAXCIR, ncirc);
  algebra::Vector3Ds circle;
  for (int i = 0; i < ncirc; ++i) {
    float fi = (2. * algebra::PI * i) / ncirc;
    circle.push_back(algebra::Vector3D(rp * std::cos(fi),
                                       rp * std::sin(fi), 0.));
  }

  SurfaceInfo surface;
  std::vector<bool> srs(CO.size(), false);
  for (unsigned i = 0; i < CO.size(); ++i) {
    std::vector<int> itnl;
    if (cube.get_neighbors(i, CO, rp, IAT, rtype, itnl)) {
      std::sort(itnl.begin(), itnl.end());
      handle_atom(fout, i, d, itnl, CO, rp, srs, up, yon_probes, circle,
                  IAT, rtype, attyp_info, beforept, surface);
    }
  }
  return surface;
}

void get_victim_probes(const std::vector<YonProbe> &yon_probes,
                       std::vector<ProbePoint> &beforept, float rp,
                       float radmax, std::vector<int> &victims)
{
  // NO VICTIM PROBES IF NO YON PROBES
  if (yon_probes.size() == 0) return;

  // Probe diameter
  float dp = 2. * rp;

  YonCube cube(yon_probes, rp, dp, radmax);

  int ivic = 0;
  for (std::vector<ProbePoint>::iterator pit = beforept.begin();
       pit != beforept.end(); ++pit, ++ivic) {
    if (pit->type == YON) continue;

    // CHECK IF PROBE TOO FAR FROM SYMMETRY ELEMENT FOR POSSIBLE OVERLAP
    if (pit->to_center.get_squared_magnitude() > dp*dp) continue;

    // LOOK FOR OVERLAP WITH ANY YON PROBE IN THE SAME MOLECULE
    if (cube.probe_overlap(*pit, yon_probes)) {
      victims.push_back(ivic);
      pit->type = VICTIM;
    }
  }
}

void get_eaten_points(const std::vector<YonProbe> &yon_probes,
                      const std::vector<ProbePoint> &beforept,
                      float dp2, const ProbePoint &probe, unsigned &neat,
                      unsigned &nyeat, const std::vector<int> &victims,
                      algebra::Vector3Ds &eat, int &pi)
{
  neat = nyeat = 0;
  eat.resize(0);
  if (yon_probes.size() == 0) return;

  pi++;
  // DETERMINE IF PROBE IS A YON OR VICTIM PROBE
  if (probe.type == OTHER) {
    return;
  }

  // CHECK THIS VICTIM OR YON PROBE AGAINST ALL YON PROBES
  for (unsigned j = 0; j < yon_probes.size(); ++j) {
    if (algebra::get_squared_distance(probe.position,
                                      yon_probes[j].center) >= dp2) continue;
    if (probe.to_center * yon_probes[j].altitude >= 0.) continue;

    // THIS YON PROBE COULD EAT SOME OF THE PROBE'S POINTS
    neat++;
    nyeat++;
    eat.push_back(yon_probes[j].center);
  }

  // ONLY YON PROBES CAN HAVE THEIR POINTS EATEN BY VICTIMS
  if (probe.type != YON) return;

  // CHECK THIS YON PROBE AGAINST ALL VICTIM PROBES
  for (unsigned j = 0; j < victims.size(); ++j) {
    const ProbePoint &victim = beforept[victims[j]];
    if (algebra::get_squared_distance(probe.position,
                                      victim.position) >= dp2) continue;
    if (probe.to_center * victim.to_center >= 0.) continue;
    // THIS VICTIM PROBE COULD EAT SOME OF THE PROBE'S POINTS
    neat++;
    eat.push_back(victim.position);
  }
}

void check_eaten_points(std::ostream &fout,
                        const std::vector<YonProbe> &yon_probes,
                        const std::vector<ProbePoint> &beforept,
                        float rp,
                        const std::vector<int> &victims,
                        SurfaceInfo &surface)
{
  float rp2 = rp * rp;
  float dp = rp * 2.;
  float dp2 = dp * dp;
  int pi = 0;
  for (std::vector<ProbePoint>::const_iterator pit = beforept.begin();
       pit != beforept.end(); ++pit) {
    unsigned neat, nyeat;
    algebra::Vector3Ds eat;
    get_eaten_points(yon_probes, beforept, dp2, *pit, neat, nyeat, victims,
                     eat, pi);

    // READ THE SURFACE POINTS BELONGING TO THE PROBE
    for (std::vector<SurfacePoint>::const_iterator sit = pit->points.begin();
         sit != pit->points.end(); ++sit) {
      // CHECK SURFACE POINT AGAINST ALL EATERS OF THIS PROBE
      bool point_eaten = false;
      for (unsigned k = 0; k < eat.size(); ++k) {
        // VICTIM PROBES CANNOT EAT NON-YON POINTS OF YON PROBES
        if (!(pit->type == YON && !sit->yon && k >= nyeat)
            && algebra::get_squared_distance(eat[k], sit->s) < rp2) {
          point_eaten = true;
          break;
        }
      }

      if (!point_eaten) {
        algebra::Vector3D outvec = (pit->position - sit->s) / rp;
        // REENTRANT SURFACE POINT
        surface.npoints++;
        surface.area += sit->area;
        fout << std::setw(5) << sit->n1+1
             << std::setw(5) << sit->n2+1
             << std::setw(5) << sit->n3+1
             << std::fixed
             << std::setw(8) << std::setprecision(3) << sit->s[0]
             << std::setw(8) << std::setprecision(3) << sit->s[1]
             << std::setw(8) << std::setprecision(3) << sit->s[2]
             << std::setw(8) << std::setprecision(3) << sit->area
             << std::setw(7) << std::setprecision(3) << outvec[0]
             << std::setw(7) << std::setprecision(3) << outvec[1]
             << std::setw(7) << std::setprecision(3) << outvec[2]
             << "  0.500" << std::endl;
      } else {
        if (pit->ishape == 2) {
          surface.nlost_saddle++;
        } else {
          surface.nlost_concave++;
        }
      }
    }
  }
}

SurfaceInfo generate_reentrant_surface(std::ostream &fout,
                                       const std::vector<YonProbe> &yon_probes,
                                       std::vector<ProbePoint> &beforept,
                                       float rp, float radmax)
{
  SurfaceInfo surface;
  std::vector<int> victims;
  get_victim_probes(yon_probes, beforept, rp, radmax, victims);
  IMP_LOG_VERBOSE( yon_probes.size() << " yon and " << victims.size()
          << " victim probes" << std::endl);
  check_eaten_points(fout, yon_probes, beforept, rp, victims, surface);
  return surface;
}

void msdots(std::ostream &fout,
            float d, float rp,
            const std::vector<float> &rtype,
            const algebra::Vector3Ds &CO,
            const std::vector<int> &IAT)
{
  IMP_INTERNAL_CHECK(rp >= 0., "Negative probe radius: " << rp);
  float radmax = 0.;

  std::vector<AtomTypeInfo> attyp_info(rtype.size());

  // Read atom types
  for (unsigned n = 0; n < rtype.size(); ++n) {
    IMP_INTERNAL_CHECK(rtype[n] >= 0., "Negative atom radius: " << n << " "
                                       << rtype[n]);
    radmax = std::max(radmax, rtype[n]);
    // number of unit vectors depends on sphere area and input density
    int nua = std::max(1,
                  static_cast<int>((4.*algebra::PI*rtype[n]*rtype[n]) * d));
    nua = std::min(nua, MAXSPH);

    // create unit vector arrays
    genun(attyp_info[n].ua, nua);
    // compute extended vectors for later probe placement
    attyp_info[n].eva.resize(attyp_info[n].ua.size());
    for (unsigned isph = 0; isph < attyp_info[n].ua.size(); ++isph) {
      attyp_info[n].eva[isph] = (rtype[n] + rp) * attyp_info[n].ua[isph];
    }
  }
  IMP_LOG_VERBOSE( "Calculation of surface of " << CO.size()
          << " atoms, with surface point density " << d
          << " and probe radius " << rp << std::endl);

  std::vector<YonProbe> yon_probes;
  std::vector<ProbePoint> beforept;
  SurfaceInfo contact_surface =
      generate_contact_surface(fout, CO, radmax, rp, d, yon_probes,
                               IAT, rtype, attyp_info, beforept);

  SurfaceInfo reentrant_surface =
      generate_reentrant_surface(fout, yon_probes, beforept, rp, radmax);

  IMP_LOG_VERBOSE( reentrant_surface.nlost_saddle << " saddle and "
          << reentrant_surface.nlost_concave
          << " concave surface points removed during non-symmetry "
          << "overlapping reentrant surface removal" << std::endl);
  IMP_LOG_VERBOSE( contact_surface.npoints << " contact, "
          << reentrant_surface.npoints << " reentrant, and "
          << contact_surface.npoints + reentrant_surface.npoints
          << " total surface points" << std::endl);
  IMP_LOG_TERSE( "Contact area: " << contact_surface.area
          << "; reentrant area: " << reentrant_surface.area
          << "; total area: " << contact_surface.area + reentrant_surface.area
          << std::endl);
}

struct LibraryEntry {
  int code;
  float Rcoul;
  char res[5], atom[5];
};

} // namespace


void write_connolly_surface(atom::Atoms atoms, base::TextOutput fn,
                            float d, float rp)
{
   char line[150];

   /* --------------- READ LIBRARY FILE --------------- */

   std::string libf = get_data_path("vdw.lib");
   FILE *flib = fopen (libf.c_str(), "r");
   if (flib) {
      IMP_LOG_VERBOSE( "Reading Score Library: " << libf << std::endl);
   } else {
      IMP_THROW("Cannot open file: " << libf, IOException);
   }

   while ((fgets(line, 90, flib)) != NULL)
      if (line[0] != '#')
         break;

   std::vector<LibraryEntry> lib;
   do
      {
      if (line[0] != '#')
         {
         struct LibraryEntry entry;
         float Rdisp;
         int LibTyp, LibFlex, LibaaTyp;
         if (sscanf (line, "%s%s%d%f%f%d%d%d", entry.res, entry.atom,
                     &entry.code, &Rdisp, &entry.Rcoul, &LibTyp,
                     &LibFlex, &LibaaTyp) == 8) {
           lib.push_back(entry);
           }
         }
      }
   while ((fgets(line, 90, flib)) != NULL);

   fclose (flib);
   IMP_LOG_VERBOSE( "Number of atom entries in the library: " << lib.size()
           << std::endl);


   std::vector<float> rvdw;
   for (unsigned li=0; li<lib.size(); li++) {
     if (lib[li].code > static_cast<int>(rvdw.size())) {
       rvdw.resize(lib[li].code);
     }
     rvdw[lib[li].code - 1] = lib[li].Rcoul;
   }

   IMP_LOG_VERBOSE( "Number of vdW radius types: " << rvdw.size() << std::endl);

   std::vector<int> IAT(atoms.size());
   algebra::Vector3Ds CO(atoms.size());
   for (unsigned pi = 0; pi < atoms.size(); ++pi) {
     CO[pi] = core::XYZ(atoms[pi]).get_coordinates();

     /* Check to see if this atom is in the library */
     const char *atom_type = atoms[pi].get_atom_type().get_string().c_str();
     const char *res_type = atom::get_residue(atoms[pi])
                                 .get_residue_type().get_string().c_str();
     bool found = false;
     for (unsigned j=0; j<lib.size(); j++) {
       if (((strcmp(res_type, lib[j].res) == 0) &&
            (strcmp(atom_type, lib[j].atom) == 0)) ||
           ((strcmp(atom_type, "OXT")==0) &&
            (strcmp(lib[j].atom, "OXT")==0))) {
         IAT[pi] = lib[j].code - 1;
         found = true;
         break;
       }
     }
     if (!found) {
       int n;
       IMP_WARN(atoms[pi] << " not found in the library!"
                "Default values used!" << std::endl);
       if (atom_type[0] == 'C')  n = 1;
       else if (atom_type[0] == 'N')  n = 2;
       else if (atom_type[0] == 'O')  n = 3;
       else if (atom_type[0] == 'S')  n = 4;
       else if (atom_type[0] == 'P')  n = 5;
       else if (!strncmp(atom_type,"FE",2))  n = 6;
       else if (!strncmp(atom_type,"MG",2))  n = 7;
       else  n = 1;
       IAT[pi]   = n-1;
     }
   }

   IMP_LOG_VERBOSE( "Total number of atoms: " << CO.size() << std::endl);

   /* --------- RUN CONNOLLY'S MOLECULAR SURFACE PROGRAM -------- */
   msdots(fn.get_stream(), d, rp, rvdw, CO, IAT);
}

IMPMULTIFIT_END_NAMESPACE
