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
#include <IMP/internal/constants.h>

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
  VectorD(Float x) {
#ifdef SWIG_WRAPPER
    IMP_check(D==1, "Need " << D << " to construct a "
              << D << "-vector.", ValueException);
#else
    BOOST_STATIC_ASSERT(D==1);
#endif
    vec_[0] = x;
  }

  //! Initialize the 2-vector from separate x,y values.
  VectorD(Float x, Float y) {
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
  VectorD(Float x, Float y, Float z) {
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
  VectorD(Float w, Float x, Float y, Float z) {
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
      vec_[i]= std::numeric_limits<Float>::quiet_NaN();
    }
#endif
  }

  //! \return A single component of this vector (0-D).
  Float operator[](unsigned int i) const {
    IMP_assert(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    return vec_[i];
  }

  //! \return A single component of this vector (0-D).
  Float& operator[](unsigned int i) {
    IMP_assert(i < D, "Invalid component of vector requested: "
               << i << " of " << D);
    return vec_[i];
  }

  //! \return the scalar product of two vectors.
  Float scalar_product(const This &o) const {
    Float ret=0;
    for (unsigned int i=0; i< D; ++i) {
      ret += vec_[i]* o.vec_[i];
    }
    return ret;
  }

  //! scalar product
  Float operator*(const This &o) const {
    return scalar_product(o);
  }

  //! product with scalar
  VectorD operator*(Float s) const {
    This ret;
    for (unsigned int i=0; i< D; ++i) {
      ret.vec_[i] = vec_[i] * s;
    }
    return ret;
  }

  //! divide by a scalar
  VectorD operator/(Float s) const {
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

  //! \return the vector product of two vectors.
  VectorD vector_product(const VectorD &vec2) const {
#ifdef SWIG_WRAPPER
    IMP_check(D==3, "Need " << D << " to perform a vector product ",
              ValueException);
#else
    BOOST_STATIC_ASSERT(D==3);
#endif
    return VectorD(vec_[1] * vec2.vec_[2] - vec_[2] * vec2.vec_[1],
                   vec_[2] * vec2.vec_[0] - vec_[0] * vec2.vec_[2],
                   vec_[0] * vec2.vec_[1] - vec_[1] * vec2.vec_[0]);
  }

  //! \return The square of the magnitude of this vector.
  Float get_squared_magnitude() const {
    return scalar_product(*this);
  }

  //! \return The magnitude of this vector.
  Float get_magnitude() const {
    return std::sqrt(get_squared_magnitude());
  }

  //! \return This vector normalized to unit length.
  VectorD get_unit_vector() const {
    Float mag = get_magnitude();
    // avoid division by zero
    mag = std::max(mag, static_cast<Float>(1e-12));
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
  VectorD& operator/=(Float f) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] /= f;
    }
    return *this;
  }

  //! Rescale the vector
  VectorD& operator*=(Float f) {
    for (unsigned int i=0; i< D; ++i) {
      vec_[i] *= f;
    }
    return *this;
  }

  void show(std::ostream &out=std::cout, std::string delim=", ") const {
    out << "(";
    for (unsigned int i=0; i< D; ++i) {
      out << vec_[i];
      if (i != D-1) {
        out << delim;
      }
    }
    out << ")";
  }

private:
  int compare(const This &o) const {
    for (unsigned int i=0; i< D; ++i) {
      if (vec_[i] < o.vec_[i]) return -1;
      else if (vec_[i] > o.vec_[i]) return 1;
    }
    return 0;
  }

  Float vec_[D];
};

template <unsigned int D>
std::ostream &operator<<(std::ostream &out, const VectorD<D> &v) {
  v.show(out);
  return out;
}


//! product with scalar
template <unsigned int D>
inline VectorD<D> operator*(Float s, const VectorD<D> &o) {
  return o*s;
}

//! create a constant vector
/** This is not the right name.
 */
template <unsigned int D>
VectorD<D> constant_vector(Float s) {
  VectorD<D> ret;
  for (unsigned int i= 0; i < D; ++i) {
    ret[i]=s;
  }
  return ret;
}


//! Generate a random vector in a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_box(const VectorD<D> &lb,
                     const VectorD<D> &ub) {
  VectorD<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    IMP_check(lb[i] < ub[i], "Box for randomize must be non-empty",
              ValueException);
    ::boost::uniform_real<> rand(lb[i], ub[i]);
    ret[i]=rand(random_number_generator);
  }
  return ret;
}

//! Generate a random vector in a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_box() {
  return random_vector_in_box(VectorD<D>(0,0,0),
                              VectorD<D>(1,1,1));
}

//! Generate a random vector in a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_sphere(const VectorD<D> &center,
                        Float radius){
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  VectorD<D> rad= constant_vector<D>(radius);
  VectorD<D> min= center - rad;
  VectorD<D> max= center + rad;
  Float norm;
  VectorD<D> ret;
  // \todo This algorithm could be more efficient.
  do {
    ret=random_vector_in_box(min, max);
    norm= (center- ret).get_magnitude();
  } while (norm > radius);
  return ret;
}

//! Generate a random vector in a unit sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_sphere(){
  return random_vector_in_sphere(VectorD<D>(0,0,0), 1);
}

//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_sphere(const VectorD<D> &center,
                        Float radius) {
  // could be made general
  BOOST_STATIC_ASSERT(D>0);
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  Float cur_radius=radius;
  VectorD<D> up;
  for (unsigned int i=D-1; i>0; --i) {
    ::boost::uniform_real<> rand(-cur_radius,cur_radius);
    up[i]= rand(random_number_generator);
    // radius of circle
    cur_radius= std::sqrt(square(cur_radius)-square(up[i]));
  }
  ::boost::uniform_int<> rand(0, 1);
  Float x= cur_radius;
  if (rand(random_number_generator)) {
    x=-x;
  }
  up[0]=x;

  IMP_assert(std::abs(up.get_magnitude() -radius) < .1,
             "Error generating vector on sphere: "
             << up << " for " << radius);
  IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);

  return center+ up;
}


//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_unit_sphere() {
  VectorD<D> v;
  for (unsigned int i=0; i < D; ++i) {
    v[i]=0;
  }
  return random_vector_on_sphere(v, 1);
}


//! compute the squared distance between two vectors
template <unsigned int D>
Float squared_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  Float d, s = 0;
  for (unsigned int i=0; i< D; ++i) {
    d = v1[i] - v2[i];
    s += d*d;
  }
  return s;
}

//! compute the distance between two vectors
template <unsigned int D>
Float distance(const VectorD<D> &v1, const VectorD<D> &v2) {
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
  s.v_.show(out, " ");
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
