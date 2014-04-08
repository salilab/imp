/**
 *  \file IMP/algebra/VectorD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_D_H
#define IMPALGEBRA_VECTOR_D_H

#include <IMP/algebra/algebra_config.h>
#include "VectorBaseD.h"
#include <IMP/base/types.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/exception.h>
#include <IMP/base/utility.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/base/random.h>
#include "algebra_macros.h"
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/range.hpp>
#include "internal/vector.h"

#include <limits>
#include <cmath>
#include <boost/random/normal_distribution.hpp>
#include <boost/static_assert.hpp>

#if IMP_HAS_CHECKS >= IMP_USAGE
#define IMP_ALGEBRA_VECTOR_CHECK check_vector()
#define IMP_ALGEBRA_VECTOR_CHECK_INDEX(i) check_index(i)
#define IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o) \
  check_compatible_vector(o);                  \
  o.check_vector()
#else
#define IMP_ALGEBRA_VECTOR_CHECK
#define IMP_ALGEBRA_VECTOR_CHECK_INDEX(i)
#define IMP_ALGEBRA_VECTOR_CHECK_COMPATIBLE(o)
#endif

IMPALGEBRA_BEGIN_NAMESPACE
//! A Cartesian vector in D-dimensions.
/** Store a vector of Cartesian coordinates. It supports all expected
    mathematical operators, including using * for the dot product.
    \see Vector3D
    \see Vector2D

    \geometry
 */
template <int D>
class VectorD : public VectorBaseD<D> {
  /* implementing it via a specialization is in order to get swig
     to only use the right constructors as well as C++. */
  IMP_ALGEBRA_VECTOR_METHODS(D);
};

template <>
class VectorD<-1> : public VectorBaseD<-1> {
  std::vector<double> get_range(double x0, double x1, double x2, double x3,
                                double x4, double x5) {
    IMP_USAGE_CHECK(x0 != std::numeric_limits<double>::max(),
                    "Bad init values");
    std::vector<double> ret;
    ret.push_back(x0);
    if (x1 != std::numeric_limits<double>::max()) ret.push_back(x1);
    if (x2 != std::numeric_limits<double>::max()) ret.push_back(x2);
    if (x3 != std::numeric_limits<double>::max()) ret.push_back(x3);
    if (x4 != std::numeric_limits<double>::max()) ret.push_back(x4);
    if (x5 != std::numeric_limits<double>::max()) ret.push_back(x5);
    return ret;
  }

 public:
  IMP_ALGEBRA_VECTOR_METHODS(-1);
  explicit VectorD(double x0, double x1 = std::numeric_limits<double>::max(),
                   double x2 = std::numeric_limits<double>::max(),
                   double x3 = std::numeric_limits<double>::max(),
                   double x4 = std::numeric_limits<double>::max(),
                   double x5 = std::numeric_limits<double>::max())
      : P(get_range(x0, x1, x2, x3, x4, x5)) {}
};

template <>
class VectorD<1> : public VectorBaseD<1> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(1);

  //! Initialize the 1-vector from its value.
  VectorD(double x) { P::operator[](0) = x; }
};
template <>
class VectorD<2> : public VectorBaseD<2> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(2);

  VectorD(double x, double y) {
    P::operator[](0) = x;
    P::operator[](1) = y;
  }
};
template <>
class VectorD<3> : public VectorBaseD<3> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(3);

  VectorD(double x, double y, double z) {
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
  }
};
template <>
class VectorD<4> : public VectorBaseD<4> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(4);

  VectorD(double x0, double x1, double x2, double x3) {
    P::operator[](0) = x0;
    P::operator[](1) = x1;
    P::operator[](2) = x2;
    P::operator[](3) = x3;
  }
};
template <>
class VectorD<5> : public VectorBaseD<5> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(5);

  VectorD(double x0, double x1, double x2, double x3, double x4) {
    P::operator[](0) = x0;
    P::operator[](1) = x1;
    P::operator[](2) = x2;
    P::operator[](3) = x3;
    P::operator[](4) = x4;
  }
};
template <>
class VectorD<6> : public VectorBaseD<6> {
 public:
  IMP_ALGEBRA_VECTOR_METHODS(6);

  VectorD(double x0, double x1, double x2, double x3, double x4, double x5) {
    P::operator[](0) = x0;
    P::operator[](1) = x1;
    P::operator[](2) = x2;
    P::operator[](3) = x3;
    P::operator[](4) = x4;
    P::operator[](5) = x5;
  }
};

#ifndef IMP_DOXYGEN

template <int D>
inline std::ostream &operator<<(std::ostream &out, const VectorD<D> &v) {
  v.show(out);
  return out;
}

#ifndef SWIG
// SWIG 3.0.0 cannot parse operator>>
template <int D>
inline std::istream &operator>>(std::istream &in, VectorD<D> &v) {
  for (unsigned int i = 0; i < D; ++i) {
    in >> v[i];
  }
  return in;
}
#endif

#endif

//! lexicographic comparison of two vectors
/** Note that this is not very reliable and probably should not be used.
    See VectorD
 */
template <int D>
inline int compare(const VectorD<D> &a, const VectorD<D> &b) {
  IMP_USAGE_CHECK(a.get_dimension() == b.get_dimension(),
                  "Dimensions don't match.");
  for (unsigned int i = 0; i < a.get_dimension(); ++i) {
    if (a[i] < b[i])
      return -1;
    else if (a[i] > b[i])
      return 1;
  }
  return 0;
}

/** See VectorD */
template <int D>
inline VectorD<D> operator*(double s, VectorD<D> o) {
  return o *= s;
}

//! compute the squared distance between two vectors
/** See VectorD
 */
template <int D>
inline double get_squared_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  return (v1 - v2).get_squared_magnitude();
}

//! compute the distance between two vectors
/** See VectorD
 */
template <int D>
inline double get_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  return std::sqrt(get_squared_distance(v1, v2));
}

//! Return the basis vector for the given coordinate
/** Return the unit vector pointing in the direction of the requested
    coordinate. That is
    \code
    get_basis_vector_d<3>(2)== Vector3D(0,0,1);
    \endcode
    See VectorD
 */
template <int D>
inline VectorD<D> get_basis_vector_d(unsigned int coordinate) {
  IMP_USAGE_CHECK(coordinate < D, "There are only " << D << " basis vectors");
  double vs[D];
  for (unsigned int i = 0; i < D; ++i) {
    if (i == coordinate)
      vs[i] = 1;
    else
      vs[i] = 0;
  }
  return VectorD<D>(vs, vs + D);
}

//! Return a dynamically sized basis vector
inline VectorD<-1> get_basis_vector_kd(int D, unsigned int coordinate) {
  IMP_USAGE_CHECK(D > 0, "D must be positive");
  IMP_USAGE_CHECK(coordinate < static_cast<unsigned int>(D),
                  "There are only " << D << " basis vectors");
  boost::scoped_array<double> vs(new double[D]);
  for (int i = 0; i < D; ++i) {
    if (i == static_cast<int>(coordinate))
      vs[i] = 1;
    else
      vs[i] = 0;
  }
  return VectorD<-1>(vs.get(), vs.get() + D);
}

//! Return a vector of zeros
template <int D>
inline VectorD<D> get_zero_vector_d() {
  IMP_USAGE_CHECK(D > 0, "D must be positive");
  VectorD<D> ret;
  for (int i = 0; i < D; ++i) {
    ret[i] = 0;
  }
  return ret;
}

//! Return a dynamically sized vector of zeros
template <int D>
inline VectorD<D> get_zero_vector_kd(int Di) {
  IMP_USAGE_CHECK(D == Di, "D must be positive");
  IMP_UNUSED(Di);
  return get_zero_vector_d<D>();
}

//! Return a dynamically sized vector of zeros
inline VectorD<-1> get_zero_vector_kd(int D) {
  IMP_USAGE_CHECK(D > 0, "D must be positive");
  Floats vs(D, 0);
  return VectorD<-1>(vs.begin(), vs.end());
}

//! Return a vector of ones (or another constant)
template <int D>
inline VectorD<D> get_ones_vector_d(double v = 1) {
  IMP_USAGE_CHECK(D > 0, "D must be positive");
  VectorD<D> ret;
  for (unsigned int i = 0; i < D; ++i) {
    ret[i] = v;
  }
  return ret;
}

//! Return a vector of ones (or another constant)
/** Di must equal D. */
template <int D>
inline VectorD<D> get_ones_vector_kd(unsigned int Di, double v = 1) {
  IMP_USAGE_CHECK(D == Di, "D must be equal");
  IMP_UNUSED(Di);
  return get_ones_vector_d<D>(v);
}

//! Return a vector of ones (or another constant)
inline VectorD<-1> get_ones_vector_kd(unsigned int D, double v = 1) {
  IMP_USAGE_CHECK(D > 0, "D must be positive");
  boost::scoped_array<double> vv(new double[D]);
  for (unsigned int i = 0; i < D; ++i) {
    vv[i] = v;
  }
  return VectorD<-1>(vv.get(), vv.get() + D);
}

#ifndef SWIG

/** \name Norms
    We define a number of standard, \f$L^p\f$, norms on VectorD.
    - \f$L^1\f$ is the Manhattan distance, the sum of the components
    - \f$L^2\f$ is the standard Euclidean length
    - \f$L^{\inf}\f$ is the maximum of the components
    @{
*/

template <int D>
inline double get_l2_norm(const VectorD<D> &v) {
  return v.get_magnitude();
}

template <int D>
inline double get_l1_norm(const VectorD<D> &v) {
  double n = std::abs(v[0]);
  for (unsigned int i = 1; i < v.get_dimension(); ++i) {
    n += std::abs(v[i]);
  }
  return n;
}

template <int D>
inline double get_linf_norm(const VectorD<D> &v) {
  double n = std::abs(v[0]);
  for (unsigned int i = 1; i < v.get_dimension(); ++i) {
    n = std::max(n, std::abs(v[i]));
  }
  return n;
}

/** @} */

#ifndef IMP_DOXYGEN

template <int D>
struct SpacesIO {
  const VectorD<D> &v_;
  SpacesIO(const VectorD<D> &v) : v_(v) {}
};

template <int D>
struct CommasIO {
  const VectorD<D> &v_;
  CommasIO(const VectorD<D> &v) : v_(v) {}
};
template <int D>
inline std::ostream &operator<<(std::ostream &out, const SpacesIO<D> &s) {
  s.v_.show(out, " ", false);
  return out;
}
template <int D>
inline std::ostream &operator<<(std::ostream &out, const CommasIO<D> &s) {
  s.v_.show(out, ", ", false);
  return out;
}

//! Use this before outputing to delimited vector entries with a space
/** std::cout << spaces_io(v);
    produces "1.0 2.0 3.0"
    See VectorD
 */
template <int D>
inline SpacesIO<D> spaces_io(const VectorD<D> &v) {
  return SpacesIO<D>(v);
}

//! Use this before outputing to delimited vector entries with a comma
/** std::cout << commas_io(v);
    produces "1.0, 2.0, 3.0"
    See VectorD
 */
template <int D>
inline CommasIO<D> commas_io(const VectorD<D> &v) {
  return CommasIO<D>(v);
}
#endif  // doxygen

#endif  // swig
/** 1D vector typedef for swig */
typedef VectorD<1> Vector1D;
/** 1D vectors typedef for swig */
typedef base::Vector<VectorD<1> > Vector1Ds;
/** 2D vector typedef for swig */
typedef VectorD<2> Vector2D;
/** 2D vectors typedef for swig */
typedef base::Vector<VectorD<2> > Vector2Ds;
/** 3D vector typedef for swig */
typedef VectorD<3> Vector3D;
/** 3D vectors typedef for swig */
typedef base::Vector<VectorD<3> > Vector3Ds;
/** 4D vector typedef for swig */
typedef VectorD<4> Vector4D;
/** 4D vectors typedef for swig */
typedef base::Vector<VectorD<4> > Vector4Ds;
/** 5D vector typedef for swig */
typedef VectorD<5> Vector5D;
/** 5D vectors typedef for swig */
typedef base::Vector<VectorD<5> > Vector5Ds;
/** 6D vector typedef for swig */
typedef VectorD<6> Vector6D;
/** 6D vector typedef for swig */
typedef base::Vector<VectorD<6> > Vector6Ds;
/** KD vector typedef for swig */
typedef VectorD<-1> VectorKD;
/** KD vectors typedef for swig */
typedef base::Vector<VectorD<-1> > VectorKDs;

#ifndef SWIG
/** See VectorD \genericgeometry */
template <class C>
inline const VectorD<C::DIMENSION> &get_vector_geometry(const C &g) {
  return g;
}
/** See VectorD \genericgeometry */
template <class C, class E>
inline void set_vector_geometry(C &g, const E &v) {
  g = v;
}
#endif

/** See VectorD
    Return the vector that is the elementwise product of the two.
*/
template <int D>
inline VectorD<D> get_elementwise_product(const algebra::VectorD<D> &a,
                                          const algebra::VectorD<D> &b) {
  VectorD<D> ret(a);
  for (unsigned int i = 0; i < ret.get_dimension(); ++i) {
    ret[i] *= b[i];
  }
  return ret;
}

/** See VectorD
    Return the vector that is the elementwise product of the two.
*/
template <int D>
inline VectorD<D> get_elementwise_product(const Ints &a,
                                          const algebra::VectorD<D> &b) {
  IMP_USAGE_CHECK(a.size() == b.get_dimension(), "Dimensions don't match,");
  VectorD<D> ret(b);
  for (unsigned int i = 0; i < ret.get_dimension(); ++i) {
    ret[i] *= a[i];
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_D_H */
