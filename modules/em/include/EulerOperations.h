/**
 *  \file EulerOperations.h
 *  \brief Euler operations for EM
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EULER_OPERATIONS_H
#define IMPEM_EULER_OPERATIONS_H

#include "config.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Vector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Rotation matrix associated with given Euler angles (ZYZ convention)
/**
  Note: If your work is oriented towards performance, a rotation matrix is
  the most efficient way of performing rotations. Otherwise, it is advised
  to use the class Rotation3D.

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
    set_angles(z1,y,z2);
  }

  //! Provide the angles to form the Euler rotation matrix
  //! This function actually builds the matrix.
  /**
  * \param[in] Rot First Euler angle (radians) defining the rotation (Z axis)
  * \param[in] Tilt Second Euler angle (radians) defining the rotation (Y axis)
  * \param[in] Psi Third Euler angle (radians) defining the rotation (Z axis)
  */
  void set_angles(Float Rot,Float Tilt,Float Psi) {
    Float c1 = cos(Rot);
    Float c2 = cos(Tilt);
    Float c3 = cos(Psi);
    Float s1 = sin(Rot);
    Float s2 = sin(Tilt);
    Float s3 = sin(Psi);

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
  algebra::Vector3D rotate(const algebra::Vector3D &o) const {
  return algebra::Vector3D(
                  (*this)(0,0)*o[0]+(*this)(0,1)*o[1]+(*this)(0,2) * o[2],
                  (*this)(1,0)*o[0]+(*this)(1,1)*o[1]+(*this)(1,2) * o[2],
                  (*this)(2,0)*o[0]+(*this)(2,1)*o[1]+(*this)(2,2) * o[2]);
  }

  //! Returns a Rotation3D class with the rotation stored in this matrix.
  algebra::Rotation3D convert_to_rotation3D() {
    return algebra::rotation_from_matrix(
                                      (*this)(0,0),(*this)(0,1),(*this)(0, 2),
                                      (*this)(1,0),(*this)(1,1),(*this)(1, 2),
                                      (*this)(2,0),(*this)(2,1),(*this)(2, 2));
  }

}; // EulerMatrixZYZ


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EULER_OPERATIONS_H */
