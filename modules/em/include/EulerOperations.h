/**
 *  \file EulerOperations.h
 *  \brief Euler operations for EM
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EULER_OPERATIONS_H
#define IMPEM_EULER_OPERATIONS_H

#include "config.h"
#include "IMP/math_macros.h"
#include "IMP/core/Rotation3D.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/ector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Rotation matrix associated with Euler Angles,according to the convention ZYZ
/**
  If your work is oriented towards performance, a rotation matrix is the most
  efficient way of performing rotations. Otherwise, it is advised to use the
  class Rotation3D.

  This class requires the angles to be given in radians, and the convention
  used is that the rotations are performed rotating counterclockwise (right
  hand side convention).

*/
class EulerMatrixZYZ: public algebra::Matrix2D<Float>
{
public:
  //! Empty constructor
  EulerMatrixZYZ() : algebra::Matrix2D<Float>(3, 3) {
  }

  //! Constructor given the angles (in Radians)
  EulerMatrixZYZ(Float z1,Float y,Float z2) : algebra::Matrix2D<Float>(3,3) {
    Float c1 = cos(z1);
    Float c2 = cos(y);
    Float c3 = cos(z2);
    Float s1 = sin(z1);
    Float s2 = sin(y);
    Float s3 = sin(z2);

    (*this)(0, 0) = c1 * c2 * c3 - s1 * s3;
    (*this)(0, 1) = (-1.0) * c2 * c3 * s1 - c1 * s3;
    (*this)(0, 2) = c3 * s2;
    (*this)(1, 0) = c3 * s1 + c1 * c2 * s3;
    (*this)(1, 1) = c1 * c3 - c2 * s1 * s3;
    (*this)(1, 2) = s2 * s3;
    (*this)(2, 0) = (-1.0) * c1 * s2;
    (*this)(2, 1) = s1 * s2;
    (*this)(2, 2) = c2;
  }

  //! Rotates a vector. No check of correction of the rotation matrix is done.
  //! It must have been properly initialized or set.
  Vector3D rotate(const Vector3D &o) const {
  return Vector3D((*this)(0,0)*o[0]+(*this)(0,1)*o[1]+(*this)(0,2) * o[2],
                  (*this)(1,0)*o[0]+(*this)(1,1)*o[1]+(*this)(1,2) * o[2],
                  (*this)(2,0)*o[0]+(*this)(2,1)*o[1]+(*this)(2,2) * o[2]);
  }

  //! Returns a Rotation3D class with the rotation stored in this matrix.
  core::Rotation3D convert_to_rotation3D() {
    return core::rotation_from_matrix((*this)(0,0),(*this)(0,1),(*this)(0, 2),
                                      (*this)(1,0),(*this)(1,1),(*this)(1, 2),
                                      (*this)(2,0),(*this)(2,1),(*this)(2, 2));
  }

}; // EulerMatrixZYZ


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EULER_OPERATIONS_H */
