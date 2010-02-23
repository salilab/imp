/**
 *  \file VectorD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_D_H
#define IMPALGEBRA_VECTOR_D_H

#include "config.h"
#include <IMP/macros.h>
#include <IMP/exception.h>
#include <IMP/utility.h>
#include <boost/static_assert.hpp>

#include <vector>
#include <limits>
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE
//! A Cartesian vector in D-dimensions.
/** Store a vector of Cartesian coordinates.
    \see VectorD<3>
    \see VectorD<2>

    \geometry
 */
template <unsigned int D>
class VectorD
{
  void check_vector() const {
    for (unsigned int i=0; i< D; ++i) {
      IMP_USAGE_CHECK(!is_nan(vec_[i]),
                "Attempt to use uninitialized vector.");
    }
  }
public:
  // public for swig
#ifndef IMP_DOXYGEN
  typedef VectorD<D> This;
#endif
  /** The distance between b and e must be equal to D.
   */
  template <class It>
  VectorD(It b, It e) {
    IMP_INTERNAL_CHECK(std::distance(b,e) == D,
               "The size of the range must match the dimension");
    std::copy(b,e, vec_);
  }

  //! Initialize the 1-vector from its value.
  explicit VectorD(double x) {
  /* Note that MSVC gets confused with static asserts if we try to subclass
     VectorD, as we do for example in the various IMP::display Geometry
     subclasses, so replace with runtime checks. */
#if defined(IMP_SWIG_WRAPPER) || defined(_MSC_VER)
    IMP_USAGE_CHECK(D==1, "Need " << D << " to construct a "
              << D << "-vector.");
#else
    BOOST_STATIC_ASSERT(D==1);
#endif
    vec_[0] = x;
  }

  //! Initialize a 2-vector from separate x,y values.
  VectorD(double x, double y) {
#if defined(IMP_SWIG_WRAPPER) || defined(_MSC_VER)
    IMP_USAGE_CHECK(D==2, "Need " << D << " to construct a "
              << D << "-vector.");
#else
    BOOST_STATIC_ASSERT(D==2);
#endif
    vec_[0] = x;
    vec_[1] = y;
  }

  //! Initialize a 3-vector from separate x,y,z values.
  VectorD(double x, double y, double z) {
#ifdef IMP_SWIG_WRAPPER
    IMP_USAGE_CHECK(D==3, "Need " << D << " to construct a "
              << D << "-vector.");
#else
    BOOST_STATIC_ASSERT(D==3);
#endif
    vec_[0] = x;
    vec_[1] = y;
    vec_[2] = z;
  }

  //! Initialize a 4-vector from separate w,x,y,z values.
  VectorD(double x0, double x1, double x2, double x3) {
#if defined(IMP_SWIG_WRAPPER) || defined(_MSC_VER)
    IMP_USAGE_CHECK(D==4, "Need " << D << " to construct a "
              << D << "-vector.");
#else
    BOOST_STATIC_ASSERT(D==4);
#endif
    vec_[0] = x0;
    vec_[1] = x1;
    vec_[2] = x2;
    if (D==4) {
      // suppress warning.
      vec_[3] = x3;
    }
  }

  //! Default constructor
  VectorD() {
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< D; ++i) {
      vec_[i]= std::numeric_limits<double>::quiet_NaN();
    }
#endif
  }
  /** Return the ith Cartesian coordinate. In 3D use [0] to get
      the x coordinate etc.*/
  double operator[](unsigned int i) const {
    IMP_INTERNAL_CHECK(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    check_vector();
    return vec_[i];
  }
  /** Return the ith Cartesian coordinate. In 3D use [0] to get
      the x coordinate etc. */
  double& operator[](unsigned int i) {
    IMP_INTERNAL_CHECK(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    return vec_[i];
  }

  double scalar_product(const VectorD<D> &o) const {
    check_vector();
    double ret=0;
    for (unsigned int i=0; i< D; ++i) {
      ret += vec_[i]* o.vec_[i];
    }
    return ret;
  }

  double get_squared_magnitude() const {
    return scalar_product(*this);
  }

  double get_magnitude() const {
    return std::sqrt(get_squared_magnitude());
  }

  VectorD get_unit_vector() const {
    double mag = get_magnitude();
    // avoid division by zero
    mag = std::max(mag, static_cast<double>(1e-12));
    return operator/(mag);
  }

  double operator*(const VectorD<D> &o) const {
    return scalar_product(o);
  }

  VectorD operator*(double s) const {
    check_vector();
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] * s;
    }
    return ret;
  }

  VectorD operator/(double s) const {
    check_vector();
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] / s;
    }
    return ret;
  }

  VectorD operator-() const {
    check_vector();
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = -vec_[i];
    }
    return ret;
  }

  VectorD operator-(const VectorD &o) const {
    check_vector(); o.check_vector();
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] - o.vec_[i];
    }
    return ret;
  }

  VectorD operator+(const VectorD &o) const {
    check_vector(); o.check_vector();
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] + o.vec_[i];
    }
    return ret;
  }

  VectorD& operator+=(const VectorD &o) {
    check_vector(); o.check_vector();
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] += o[i];
    }
    return *this;
  }

  VectorD& operator-=(const VectorD &o) {
    check_vector(); o.check_vector();
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] -= o[i];
    }
    return *this;
  }

  VectorD& operator/=(double f) {
    check_vector();
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] /= f;
    }
    return *this;
  }

  VectorD& operator*=(double f) {
    check_vector();
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] *= f;
    }
    return *this;
  }

  void show(std::ostream &out=std::cout, std::string delim=", ",
            bool parens=true) const {
    check_vector();
    if (parens) out << "(";
    for (unsigned int i=0; i< D; ++i) {
      out << vec_[i];
      if (i != D-1) {
        out << delim;
      }
    }
    if (parens) out << ")";
  }

#ifndef IMP_DOXYGEN
  std::string __str__() const {
    std::ostringstream oss;
    show(oss);
    return oss.str();
  }
#endif

  typedef double* CoordinateIterator;
  CoordinateIterator coordinates_begin() {return vec_;}
  CoordinateIterator coordinates_end() {return vec_+D;}
#ifndef SWIG
  typedef const double* CoordinateConstIterator;
  CoordinateConstIterator coordinates_begin() const {return vec_;}
  CoordinateConstIterator coordinates_end() const {return vec_+D;}
#endif

#ifndef IMP_DOXYGEN
  const double *get_data() const {return vec_;}
#endif
private:

  double vec_[D];
};

#ifndef IMP_DOXYGEN

template <unsigned int D>
std::ostream &operator<<(std::ostream &out, const VectorD<D> &v) {
  v.show(out);
  return out;
}

template <unsigned int D>
std::istream &operator>>(std::istream &in, VectorD<D> &v) {
  for (unsigned int i=0; i< D; ++i) {
    in >> v[i];
  }
  return in;
}

#endif

//! lexicographic comparison of two vectors
/** \relatesalso VectorD
 */
template <unsigned int D>
int compare(const VectorD<D> &a, const VectorD<D> &b) {
  for (unsigned int i=0; i< D; ++i) {
    if (a[i] < b[i]) return -1;
    else if (a[i] > b[i]) return 1;
  }
  return 0;
}

/** \relatesalso VectorD */
template <unsigned int D>
inline VectorD<D> operator*(double s, const VectorD<D> &o) {
  return o*s;
}

//! compute the squared distance between two vectors
/** \relatesalso VectorD
 */
template <unsigned int D>
double get_squared_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  double d, s = 0;
  for (unsigned int i=0; i< D; ++i) {
    d = v1[i] - v2[i];
    s += d*d;
  }
  return s;
}

//! compute the distance between two vectors
/** \relatesalso VectorD
 */
template <unsigned int D>
double get_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  return std::sqrt(get_squared_distance(v1, v2));
}

//! Return the basis vector for the given coordinate
/** Return the unit vector pointing in the direction of the requested
    coordinate. That is
    \code
    get_basis_vector_d<3>(2)== VectorD<3>(0,0,1);
    \endcode
    \relatesalso VectorD
 */
template <unsigned int D>
VectorD<D> get_basis_vector_d(unsigned int coordinate) {
  IMP_USAGE_CHECK(coordinate<D, "There are only " << D << " basis vectors");
  double vs[D];
  for (unsigned int i=0; i< D; ++i) {
    if (i==coordinate) vs[i]=1;
    else vs[i]=0;
  }
  return VectorD<D>(vs, vs+D);
}

//! Return a vector of zeros
template <unsigned int D>
VectorD<D> get_zero_vector_d() {
  double vs[D]={0};
  return VectorD<D>(vs, vs+D);
}


//! Return a vector of ones (or another constant)
template <unsigned int D>
VectorD<D> get_ones_vector_d(double v=1) {
  VectorD<D> vv;
  for (unsigned int i=0; i< D; ++i) {
    vv[i]=v;
  }
  return vv;
}


#ifndef SWIG

#ifndef IMP_DOXYGEN
template <unsigned int D>
struct SpacesIO
{
  const VectorD<D> &v_;
  SpacesIO(const VectorD<D> &v): v_(v){}
};

template <unsigned int D>
struct CommasIO
{
  const VectorD<D> &v_;
  CommasIO(const VectorD<D> &v): v_(v){}
};
template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out, const SpacesIO<D> &s)
{
  s.v_.show(out, " ", false);
  return out;
}
template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out, const CommasIO<D> &s)
{
  s.v_.show(out, ", ", false);
  return out;
}
#endif
/** \name Norms
    We define a number of standard, \f$L^p\f$, norms on VectorD.
    - \f$L^1\f$ is the Manhattan distance, the sum of the components
    - \f$L^2\f$ is the standard Euclidean length
    - \f$L^\inf\f$ is the maximum of the components
    @{
*/

template <unsigned int D>
double get_l2_norm(const VectorD<D> &v) {
  return v.get_magnitude();
}

template <unsigned int D>
double get_l1_norm(const VectorD<D> &v) {
  double n=std::abs(v[0]);
  for (unsigned int i=1; i< D; ++i) {
    n+= std::abs(v[i]);
  }
  return n;
}

template <unsigned int D>
double get_linf_norm(const VectorD<D> &v) {
  double n=std::abs(v[0]);
  for (unsigned int i=1; i< D; ++i) {
    n= std::max(n, std::abs(v[i]));
  }
  return n;
}

/** @} */

//! Use this before outputing to delimited vector entries with a space
/** std::cout << spaces_io(v);
    produces "1.0 2.0 3.0"
    \relatesalso VectorD
 */
template <unsigned int D>
SpacesIO<D> spaces_io(const VectorD<D> &v) {
  return SpacesIO<D>(v);
}




//! Use this before outputing to delimited vector entries with a comma
/** std::cout << commas_io(v);
    produces "1.0, 2.0, 3.0"
    \relatesalso VectorD
 */
template <unsigned int D>
CommasIO<D> commas_io(const VectorD<D> &v) {
  return CommasIO<D>(v);
}
#endif

#ifndef SWIG
typedef VectorD<2> Vector2D;
typedef std::vector<VectorD<2> > Vector2Ds;
typedef VectorD<3> Vector3D;
typedef std::vector<VectorD<3> > Vector3Ds;
typedef VectorD<4> Vector4D;
typedef std::vector<VectorD<4> > Vector4Ds;
#endif

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_D_H */
