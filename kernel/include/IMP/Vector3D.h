/**
 *  \file Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VECTOR_3D_H
#define __IMP_VECTOR_3D_H

#include "IMP_config.h"
#include "base_types.h"

#include <cmath>

namespace IMP
{

//! Simple 3D vector class
/** \ingroup helper
 */
class IMPDLLEXPORT Vector3D
{
public:
  //! Initialize the vector from separate x,y,z values.
  Vector3D(Float x, Float y, Float z) {
    vec_[0] = x;
    vec_[1] = y;
    vec_[2] = z;
  }

  //! Default constructor
  Vector3D() {}

  //! \return A single component of this vector (0-2).
  Float operator[](unsigned int i) const {
    IMP_assert(i < 3, "Invalid component of vector requested");
    return vec_[i];
  }

  //! \return the scalar product of two vectors.
  /** \param[in] vec2 The other vector to use in the product.
   */
  Float scalar_product(const Vector3D &vec2) const {
    return vec_[0] * vec2.vec_[0] + vec_[1] * vec2.vec_[1]
           + vec_[2] * vec2.vec_[2];
  }

  //! scalar product
  Float operator*(const Vector3D &o) const {
    return scalar_product(o);
  }

  //! \return the vector product of two vectors.
  /** \param[in] vec2 The other vector to use in the product.
   */
  Vector3D vector_product(const Vector3D &vec2) const {
    return Vector3D(vec_[1] * vec2.vec_[2] - vec_[2] * vec2.vec_[1],
                    vec_[2] * vec2.vec_[0] - vec_[0] * vec2.vec_[2],
                    vec_[0] * vec2.vec_[1] - vec_[1] * vec2.vec_[0]);
  }

  //! \return The square of the magnitude of this vector.
  Float get_squared_magnitude() const {
    return vec_[0] * vec_[0] + vec_[1] * vec_[1] + vec_[2] * vec_[2];
  }

  //! \return The magnitude of this vector.
  Float get_magnitude() const {
    return std::sqrt(get_squared_magnitude());
  }

  //! \return This vector normalized to unit length.
  Vector3D get_unit_vector() const {
    Float mag = get_magnitude();
    // avoid division by zero
    mag = std::max(mag, static_cast<Float>(1e-12));
    return Vector3D(vec_[0] / mag, vec_[1] / mag, vec_[2] / mag);
  }

  //! \return Difference between two vectors.
  Vector3D operator-(const Vector3D &o) const {
    return Vector3D(operator[](0)-o[0],
                    operator[](1)-o[1],
                    operator[](2)-o[2]);
  }
private:
  Float vec_[3];
};

} // namespace IMP

#endif  /* __IMP_VECTOR_3D_H */
