/**
 *  \file VectorD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_VECTOR_D_H
#define IMP_VECTOR_D_H

#include "config.h"
#include "base_types.h"
#include "macros.h"
#include "exception.h"
#include "random.h"
#include "internal/constants.h"

#include <boost/random/uniform_real.hpp>

#include <limits>
#include <cmath>

IMP_BEGIN_NAMESPACE

//! Simple D vector class
template <unsigned int D>
class VectorD
{
  bool is_default() const {return false;}
public:
  // public for swig
  typedef VectorD<D> This;

  //! Initialize the 3-vector from separate x,y,z values.
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
    vec_[2] = z;
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
    IMP_assert(i < D, "Invalid component of vector requested");
    return vec_[i];
  }

  //! \return A single component of this vector (0-D).
  Float& operator[](unsigned int i) {
    IMP_assert(i < D, "Invalid component of vector requested");
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
    BOOST_STATIC_ASSERT(D==3);
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
  BOOST_STATIC_ASSERT(D==3);
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  ::boost::uniform_real<> rand(-1,1);
  VectorD<D> up;
  up[2]= rand(random_number_generator);
  ::boost::uniform_real<> trand(0, 2*IMP::internal::PI);
  Float theta= trand(random_number_generator);
  // radius of circle
  Float r= std::sqrt(1-square(up[2]));
  up[0]= std::sin(theta)*r;
  up[1]= std::cos(theta)*r;
  IMP_assert(std::abs(up.get_magnitude() -1) < .1,
             "Error generating unit vector on sphere");
  IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);
  return center+ up*radius;
}

//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_unit_sphere() {
  return random_vector_on_sphere(VectorD<D>(0,0,0), 1);
}


//! compute the squared distance between two vectors
template <unsigned int D>
Float get_squared_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  Float d, s = 0;
  for (unsigned int i=0; i< D; ++i) {
    d = v1[i] - v2[i];
    s += d*d;
  }
  return s;
}

//! compute the distance between two vectors
template <unsigned int D>
Float get_distance(const VectorD<D> &v1, const VectorD<D> &v2) {
  return std::sqrt(get_squared_distance(v1, v2));
}

template <unsigned int D>
struct SpacesIO
{
  const VectorD<D> &v_;
  SpacesIO(const VectorD<D> &v): v_(v){}
};

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

template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out, const CommasIO<D> &s)
{
  s.v_.show(out, ", ");
  return out;
}

IMP_END_NAMESPACE

#endif  /* IMP_VECTOR_D_H */
