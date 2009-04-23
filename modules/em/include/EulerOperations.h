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

//! Stores the 3 Euler angles of a rotation in ZYZ convention
class IMPEMEXPORT EulerAnglesZYZ
{
public:
  EulerAnglesZYZ() {};

  EulerAnglesZYZ(double z1,double y, double z2) {
    _ang[0]= z1;
    _ang[1]= y;
    _ang[2]= z2;
  }

  double operator[](unsigned int i) const {
    IMP_assert(i < 3, "Invalid component of EulerAnglesZYZ requested: "
               << i << " of " << 3);
    return _ang[i];
  }

  double& operator[](unsigned int i) {
    IMP_assert(i < 3, "Invalid component of EulerAnglesZYZ requested: "
               << i << " of " << 3);
    return _ang[i];
  }

  inline double get_r() {
    return _ang[0];
  }

private:
  double _ang[3];
};


//! Rotation matrix associated with given Euler angles (ZYZ convention)
/**
  Note: If your work is oriented towards performance, a rotation matrix is
  the most efficient way of performing rotations. Otherwise, it is advised
  to use the class Rotation3D.

  This class requires the angles to be given in radians, and the convention
  used is that the rotations are performed rotating counterclockwise (right
  hand side convention).

*/
class IMPEMEXPORT EulerMatrixZYZ: public algebra::Matrix2D<double>
{
public:
  //! Empty constructor
  EulerMatrixZYZ() : algebra::Matrix2D<double>(3, 3) {
  }

  //! Constructor given the angles (z,y,z) in radians
  EulerMatrixZYZ(const double z1,const double y, const double z2) :
                                      algebra::Matrix2D<double>(3,3) {
    set_angles(z1,y,z2);
  }

  //! Constructor given the angles as EulerAnglesZYZ
  EulerMatrixZYZ(const EulerAnglesZYZ& e) : algebra::Matrix2D<double>(3,3) {
    set_angles(e[0],e[1],e[2]);
  }

  //! Provide the angles to form the Euler rotation matrix
  //! This function actually builds the matrix.
  /**
  * \param[in] Rot First Euler angle (radians) defining the rotation (Z axis)
  * \param[in] Tilt Second Euler angle (radians) defining the rotation (Y axis)
  * \param[in] Psi Third Euler angle (radians) defining the rotation (Z axis)
  */
  void set_angles(double Rot,double Tilt,double Psi) {
    double c1 = cos(Rot);
    double c2 = cos(Tilt);
    double c3 = cos(Psi);
    double s1 = sin(Rot);
    double s2 = sin(Tilt);
    double s3 = sin(Psi);

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

  //! Returns the direction associated with the two first Euler angles, Rot and
  //! Tilt
  algebra::Vector3D direction() {
    return algebra::Vector3D((*this)(2,0),(*this)(2,1),(*this)(2,2));
  }

}; // EulerMatrixZYZ


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EULER_OPERATIONS_H */
