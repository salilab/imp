/**
 *  \file rotation_operations.h
 *  \brief Classes and operations related with rotations
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_ROTATION_OPERATIONS_H
#define IMPALGEBRA_ROTATION_OPERATIONS_H

#include "config.h"
#include "utility.h"
#include "VectorD.h"
#include "Vector3D.h"
#include "Matrix2D.h"
#include "Rotation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Stores the 3 Euler angles of a 3D rotation in ZYZ convention
class EulerAnglesZYZ: public UninitializedDefault
{
public:
  EulerAnglesZYZ() {
  }

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
class EulerMatrixZYZ: public Matrix2D<double>
{
public:
  //! Empty constructor
  EulerMatrixZYZ() : Matrix2D<double>(3, 3) {
  }

  //! Constructor given the angles (z,y,z) in radians
  EulerMatrixZYZ(const double z1,const double y, const double z2) :
                                      Matrix2D<double>(3,3) {
    set_angles(z1,y,z2);
  }

  //! Constructor given the angles as EulerAnglesZYZ
  EulerMatrixZYZ(const EulerAnglesZYZ& e) : Matrix2D<double>(3,3) {
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
  Vector3D rotate(const Vector3D &o) const {
  return Vector3D(
            (*this)(0,0)*o[0]+(*this)(0,1)*o[1]+(*this)(0,2) * o[2],
            (*this)(1,0)*o[0]+(*this)(1,1)*o[1]+(*this)(1,2) * o[2],
            (*this)(2,0)*o[0]+(*this)(2,1)*o[1]+(*this)(2,2) * o[2]);
  }

  //! Returns a Rotation3D class with the rotation stored in this matrix.
  Rotation3D convert_to_rotation3D() {
    return rotation_from_matrix(
                            (*this)(0,0),(*this)(0,1),(*this)(0, 2),
                            (*this)(1,0),(*this)(1,1),(*this)(1, 2),
                            (*this)(2,0),(*this)(2,1),(*this)(2, 2));
  }

  //! Returns the direction associated with the two first Euler angles, Rot and
  //! Tilt
  Vector3D direction() {
    return Vector3D((*this)(2,0),(*this)(2,1),(*this)(2,2));
  }

}; // EulerMatrixZYZ


//! Stores a 2D rotation matrix
/**
  \note This class requires the angles to be given in radians, and the
  onvention used is that the rotations are performed rotating counterclockwise
  (right hand side convention).
**/
class RotationMatrix2D
{
public:
  RotationMatrix2D() {
    set_angle(0.0);
  };

  //! Builds the matrix for the given angle
  RotationMatrix2D(double psi) {
    set_angle(psi);
  }

  //! sets the angle for the rotation
  /**
  * \param[in] psi the angle
  */
  void set_angle(double psi) {
    _psi = psi;
    _c = cos(psi);
    _s = sin(psi);
  }

  //! rotates a 2D point
  /**
  * \param[in] o a 2D vector to be rotated
  */
  VectorD<2> rotate(const VectorD<2> &o) const {
    return rotate(o[0],o[1]);
  }

  //! rotates a 2D point
  VectorD<2> rotate(const double x,const double y) const {
    return VectorD<2>(_c*x-_s*y , _s*x+_c*y);
  }

  //! Returns the matrix for the inverse rotation
  RotationMatrix2D get_inverse() {
    return RotationMatrix2D(-_psi);
  }
private:
  double _psi; // angle
  double _c; // cosine of the angle
  double _s; // sine of the angle
};


//! Rotates a Matrix2D a given angle (in radians). The center of the matrix
//! is used as the center of rotation. The input matrix is not modified.
/**
  \param[in] m the matrix to rotate
  \param[out] result the result matrix. It cannot be input matrix.
  \param[in] ang angle
  \param[in] wrap if true, the Matrix2D is wrapped.
                      See help for bilinear_interp()
  \param[in] wrap if true, the Matrix2D is wrapped.
                      See help for bilinear_interp()
  \param[in] outside Value to apply if a rotated point falls outside the
               limits of the matrix. (default = 0.0)
**/
template<typename T>
void rotate_matrix_2D(Matrix2D<T>& m,double ang,
              Matrix2D<T>& result,
              bool wrap=false,T outside=0.0) {
  result.resize(m);
  // Save the origin and center
  std::vector<int> orig(2);
  orig[0]=m.get_start(0);
  orig[1]=m.get_start(1);
  m.centered_start();
  result.centered_start();
  // Build the rotation needed for the angle (build the inverse directly)
  RotationMatrix2D inv_rot((-1)*ang);
  // Roll over the destination matrix
  for(int i = result.get_start(0);i<=result.get_finish(0);i++) { // Y
    for(int j = result.get_start(1);j<=result.get_finish(1);j++) { // X
      // Compute the equivalent point in the original matrix
      VectorD<2> v = inv_rot.rotate((double)i,(double)j);
      // Interpolate the value from the original matrix
      result(i,j) = m.bilinear_interp(v,wrap,outside);
    }
  }
  // Restore the origins
  m.set_start(orig);
  result.set_start(orig);
}


//! Rotates a Matrix2D a given angle (in radians). The center of the matrix
//! is used as the center of rotation. The matrix is modified
/**
  \param[in] m the matrix to rotate
  \param[in] ang angle
  \param[in] wrap if true, the Matrix2D is wrapped.
                      See help for bilinear_interp()
  \param[in] outside Value to apply if a rotated point falls outside the
               limits of the matrix. (default = 0.0)
**/
template<typename T>
void auto_rotate_matrix_2D(Matrix2D<T>& m,double ang,
                           bool wrap=false,T outside=0.0) {
  Matrix2D<T> result;
  rotate_matrix_2D(m,ang,result,wrap,outside);
  m.copy(result);
}

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_OPERATIONS_H */
