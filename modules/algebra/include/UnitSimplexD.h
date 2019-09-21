/**
 *  \file IMP/algebra/UnitSimplexD.h
 *  \brief Simple unit simplex class.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_UNIT_SIMPLEX_D_H
#define IMPALGEBRA_UNIT_SIMPLEX_D_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/algebra/Triangle3D.h>
#include <IMP/algebra/GeometricPrimitiveD.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/showable_macros.h>
#include <boost/math/special_functions/gamma.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

IMPALGEBRA_BEGIN_NAMESPACE

template <int D>
class UnitSimplexBaseD : public GeometricPrimitiveD<D> {
 public:
  //! Get dimension D of embedded real space.
  virtual int get_dimension() const = 0;

  //! Get center of mass on simplex in embedded coordinates.
  /** This is equivalent to the ones vector divided by D. */
  VectorD<D> get_barycenter() const {
    double ci = 1.0 / static_cast<double>(get_dimension());
    return get_ones_vector_kd<D>(get_dimension(), ci);
  }

  //! Get whether the point is on the unit simplex
  /**
    \param[in] p Point
    \param[in] atol Absolute tolerance of error for each element. Total
                    tolerance is atol*d.
   */
  bool get_contains(
      const VectorD<D> &p,
      double atol = std::numeric_limits<double>::epsilon()) const {
    int d = p.get_dimension();
    if (d != get_dimension()) return false;
    double nrm = 0;
    for (int i = 0; i < d; ++i) {
      if (p[i] < 0) return false;
      nrm += p[i];
    }
    if (std::abs(nrm - 1) > d * atol) return false;
    return true;
  }
};

//! Represent a unit simplex embedded in D-dimensional real space.
/** The unit simplex (also known as standard simplex or probability simplex)
    is the manifold of vectors of probabilities that sum to 1:
    \f$ \Delta^{D-1} = \{ (t_1,\dots,t_D) \in \mathbb{R}^D \mid \sum_{d=1}^D t_d = 1, t_d \ge 0 \} \f$

    \geometry
 */
template <int D>
class UnitSimplexD : public UnitSimplexBaseD<D> {
 public:
  UnitSimplexD() { IMP_USAGE_CHECK(D > 0, "Dimension must be positive."); }

  //! Get dimension D of embedded real space.
  int get_dimension() const IMP_OVERRIDE { return D; }

  IMP_SHOWABLE_INLINE(UnitSimplexD<D>, { out << "UnitSimplex" << D << "D"; });
};

//! Represent a unit simplex embedded in d-dimensional real space.
/** This version's dimension is not known at compile time and can operate with
    variable-dimension vectors.
 */
template <>
class UnitSimplexD<-1> : public UnitSimplexBaseD<-1> {
 public:
  //! Construct from dimension d of embedded real space.
  UnitSimplexD(int d = 1) : d_(d) {
    IMP_USAGE_CHECK(d > 0, "Dimension must be positive.");
  }

  //! Get dimension of embedded real space.
  int get_dimension() const IMP_OVERRIDE { return d_; }

  // FIXME: SWIG doesn't seem to use this.
  IMP_SHOWABLE_INLINE(UnitSimplexD<-1>,
                      { out << "UnitSimplexKD(" << d_ << ")"; });

 private:
  int d_;
};

typedef UnitSimplexD<1> UnitSimplex1D;
IMP_VALUES(UnitSimplex1D, UnitSimplex1Ds);
typedef UnitSimplexD<2> UnitSimplex2D;
IMP_VALUES(UnitSimplex2D, UnitSimplex2Ds);
typedef UnitSimplexD<3> UnitSimplex3D;
IMP_VALUES(UnitSimplex3D, UnitSimplex3Ds);
typedef UnitSimplexD<4> UnitSimplex4D;
IMP_VALUES(UnitSimplex4D, UnitSimplex4Ds);
typedef UnitSimplexD<5> UnitSimplex5D;
IMP_VALUES(UnitSimplex5D, UnitSimplex5Ds);
typedef UnitSimplexD<6> UnitSimplex6D;
IMP_VALUES(UnitSimplex6D, UnitSimplex6Ds);
typedef UnitSimplexD<-1> UnitSimplexKD;
IMP_VALUES(UnitSimplexKD, UnitSimplexKDs);

//! Return a list of the vertices (bases) of the unit simplex.
template <int D>
inline Vector<VectorD<D> > get_vertices(const UnitSimplexD<D> &s) {
  Vector<VectorD<D> > ret;
  int d = s.get_dimension();
  for (int i = 0; i < d; ++i)
    ret.push_back(get_basis_vector_kd<D>(d, i));
  return ret;
}

inline algebra::Triangle3D get_triangle_3d(const UnitSimplex3D &s) {
  Vector3Ds ps = get_vertices(s);
  return Triangle3D(ps[0], ps[1], ps[2]);
}

//! Convert point on simplex from embedded to increasing coordinates.
/** Increasing coordinates are defined as the vector whose elements contain the
    cumulative sum of all embedded coordinates of lower indices.
 */
template <int D>
inline VectorD<D> get_increasing_from_embedded(const UnitSimplexD<D> &s,
                                               const VectorD<D> &p) {
  int d = s.get_dimension();
  IMP_USAGE_CHECK(static_cast<int>(p.get_dimension()) == d,
                  "Dimension of point must match dimension of simplex.");
  IMP_INTERNAL_CHECK(s.get_contains(p), "Input point is not on simplex");
  VectorD<D> q = get_zero_vector_kd(d);
  std::partial_sum(p.begin(), p.end(), q.begin());
  return q;
}

//! Convert point on simplex from increasing to embedded coordinates.
/** \see get_increasing_from_embedded */
template <int D>
inline VectorD<D> get_embedded_from_increasing(const UnitSimplexD<D> &s,
                                               const VectorD<D> &p) {
  int d = s.get_dimension();
  IMP_USAGE_CHECK(static_cast<int>(p.get_dimension()) == d,
                  "Dimension of point must match dimension of simplex.");
  VectorD<D> q = get_zero_vector_kd(d);
  std::adjacent_difference(p.begin(), p.end(), q.begin());
  IMP_INTERNAL_CHECK(s.get_contains(q), "Output point is not on simplex");
  return q;
}

// Perform Euclidean projection of p onto the unit simplex s.
/**
    Any negative weights are set to 0, and the unit l1-norm is enforced. The
    algorithm used to project to the simplex is described in
    arXiv:1309.1541. It finds a threshold below which all weights are set to
    0 and above which all weights shifted by the threshold sum to 1.
 */
template <int D>
inline VectorD<D> get_projected(const UnitSimplexD<D> &s, const VectorD<D> &p) {
  int d = s.get_dimension();
  IMP_USAGE_CHECK(d == static_cast<int>(p.get_dimension()),
                  "Dimension of point must match dimension of simplex.");

  if (s.get_contains(p)) return p;

  VectorD<D> u(p);
  std::sort(u.begin(), u.end(), std::greater<double>());

  Floats u_cumsum(d);
  std::partial_sum(u.begin(), u.end(), u_cumsum.begin());

  int rho = 1;
  while (rho < d) {
    if (u[rho] + (1 - u_cumsum[rho]) / (rho + 1) < 0) break;
    rho += 1;
  }
  double lam = (1 - u_cumsum[rho - 1]) / rho;

  for (int i = 0; i < d; ++i) {
    double ui = p[i] + lam;
    u[i] = ui > 0 ? ui : 0.0;
  }

  return u;
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_UNIT_SIMPLEX_D_H */
