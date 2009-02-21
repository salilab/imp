/**
 *  \file VectorD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_D_H
#define IMPALGEBRA_VECTOR_D_H

#include "config.h"
#include <IMP/base_types.h>
#include <IMP/macros.h>
#include <IMP/exception.h>
#include <IMP/random.h>
#include <IMP/utility.h>

#include <boost/random/uniform_real.hpp>
#include <boost/static_assert.hpp>

#include <limits>
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE

//! Simple D vector class
template <unsigned int D>
class VectorD
{
  bool is_default() const {return false;}
public:
  // public for swig
  typedef VectorD<D> This;

  //! Initialize the 1-vector from its value.
  VectorD(double x) {
#ifdef SWIG_WRAPPER
    IMP_check(D==1, "Need " << D << " to construct a "
              << D << "-vector.", ValueException);
#else
    BOOST_STATIC_ASSERT(D==1);
#endif
    vec_[0] = x;
  }

  //! Initialize the 2-vector from separate x,y values.
  VectorD(double x, double y) {
#ifdef SWIG_WRAPPER
    IMP_check(D==2, "Need " << D << " to construct a "
              << D << "-vector.", ValueException);
#else
    BOOST_STATIC_ASSERT(D==2);
#endif
    vec_[0] = x;
    vec_[1] = y;
  }

  //! Initialize the 2-vector from separate x,y values.
  VectorD(double x, double y, double z) {
#ifdef SWIG_WRAPPER
    IMP_check(D==3, "Need " << D << " to construct a "
              << D << "-vector.", ValueException);
#else
    BOOST_STATIC_ASSERT(D==3);
#endif
    vec_[0] = x;
    vec_[1] = y;
    vec_[2] = z;
  }

  //! Initialize the 2-vector from separate x,y values.
  VectorD(double w, double x, double y, double z) {
#ifdef SWIG_WRAPPER
    IMP_check(D==4, "Need " << D << " to construct a "
              << D << "-vector.", ValueException);
#else
    BOOST_STATIC_ASSERT(D==4);
#endif
    vec_[0] = w;
    vec_[1] = x;
    vec_[2] = y;
    vec_[3] = z;
  }

  //! Default constructor
  VectorD() {
#ifndef NDEBUG
    for (unsigned int i=0; i< D; ++i) {
      vec_[i]= std::numeric_limits<double>::quiet_NaN();
    }
#endif
  }

  //! \return A single component of this vector (0-D).
  double operator[](unsigned int i) const {
    IMP_assert(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    return vec_[i];
  }

  //! \return A single component of this vector (0-D).
  double& operator[](unsigned int i) {
    IMP_assert(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    return vec_[i];
  }

  //! \return the scalar product of two vectors.
  double scalar_product(const This &o) const {
    double ret=0;
    for (unsigned int i=0; i< D; ++i) {
      ret += vec_[i]* o.vec_[i];
    }
    return ret;
  }

  //! scalar product
  double operator*(const This &o) const {
    return scalar_product(o);
  }

  //! product with scalar
  VectorD operator*(double s) const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] * s;
    }
    return ret;
  }

  //! divide by a scalar
  VectorD operator/(double s) const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] / s;
    }
    return ret;
  }

  //! negation
  VectorD operator-() const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = -vec_[i];
    }
    return ret;
  }

  //! \return The square of the magnitude of this vector.
  double get_squared_magnitude() const {
    return scalar_product(*this);
  }

  //! \return The magnitude of this vector.
  double get_magnitude() const {
    return std::sqrt(get_squared_magnitude());
  }

  //! \return This vector normalized to unit length.
  VectorD get_unit_vector() const {
    IMP_assert(get_magnitude() != 0,
               "Cannot get a unit vector from a zero vector");
    double mag = get_magnitude();
    // avoid division by zero
    mag = std::max(mag, static_cast<double>(1e-12));
    return operator/(mag);
  }

  //! \return Difference between two vectors.
  VectorD operator-(const VectorD &o) const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] - o.vec_[i];
    }
    return ret;
  }

  //! \return Sum of two vectors.
  VectorD operator+(const VectorD &o) const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] + o.vec_[i];
    }
    return ret;
  }

  //! Accumulate the vector
  VectorD& operator+=(const VectorD &o) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] += o[i];
    }
    return *this;
  }

  //! Subtract in place
  VectorD& operator-=(const VectorD &o) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] -= o[i];
    }
    return *this;
  }

  //! Rescale the vector
  VectorD& operator/=(double f) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] /= f;
    }
    return *this;
  }

  //! Rescale the vector
  VectorD& operator*=(double f) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] *= f;
    }
    return *this;
  }

  void show(std::ostream &out=std::cout, std::string delim=", ",
            bool parens=true) const {
    if (parens) out << "(";
    for (unsigned int i=0; i< D; ++i) {
      out << vec_[i];
      if (i != D-1) {
        out << delim;
      }
    }
    if (parens) out << ")";
  }

private:

  double vec_[D];
};

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

//! lexicographic comparison of two vectors
template <unsigned int D>
int compare(const VectorD<D> &a, const VectorD<D> &b) {
  for (unsigned int i=0; i< D; ++i) {
    if (a[i] < b[i]) return -1;
    else if (a[i] > b[i]) return 1;
  }
  return 0;
}

//! product with scalar
template <unsigned int D>
inline VectorD<D> operator*(double s, const VectorD<D> &o) {
  return o*s;
}

//! compute the squared distance between two vectors
template <unsigned int D>
double squared_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  double d, s = 0;
  for (unsigned int i=0; i< D; ++i) {
    d = v1[i] - v2[i];
    s += d*d;
  }
  return s;
}

//! compute the distance between two vectors
template <unsigned int D>
double distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  return std::sqrt(squared_distance(v1, v2));
}

template <unsigned int D>
struct SpacesIO
{
  const VectorD<D> &v_;
  SpacesIO(const VectorD<D> &v): v_(v){}
};

//! Use this before outputing to delimited vector entries with a space
/** std::cout << spaces_io(v);
    produces "(1.0 2.0 3.0)"

 */
template <unsigned int D>
SpacesIO<D> spaces_io(const VectorD<D> &v) {
  return SpacesIO<D>(v);
}

template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out, const SpacesIO<D> &s)
{
  s.v_.show(out, " ", false);
  return out;
}

template <unsigned int D>
struct CommasIO
{
  const VectorD<D> &v_;
  CommasIO(const VectorD<D> &v): v_(v){}
};

//! Use this before outputing to delimited vector entries with a comma
/** std::cout << commas_io(v);
 produces "(1.0, 2.0, 3.0)"

 */
template <unsigned int D>
CommasIO<D> commas_io(const VectorD<D> &v) {
  return CommasIO<D>(v);
}

template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out, const CommasIO<D> &s)
{
  s.v_.show(out, ", ");
  return out;
}

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_D_H */
