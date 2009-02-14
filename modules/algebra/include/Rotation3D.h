/**
 *  \file Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ROTATION_3D_H
#define IMPALGEBRA_ROTATION_3D_H

#include "config.h"
#include "Vector3D.h"
#include <iostream>
#include <algorithm>
IMPALGEBRA_BEGIN_NAMESPACE

//! 3D rotation class.
/** Holds a three dimensional rotation compactly using a quaternion (4 numbers).
    Advantages using quaternion:
    1. Easy convertion between axis/angle to quaternion reprsentation
    2. Robustness to rounding errors.
    3. Is not subject to "Gimbal lock" (i.e. attempts to rotate an
       object fail to appear as expected, due to the order in which the
       rotations are performed) like Euler angles.
    4. Can be interpolated
    5. The quaternions representation does not harm the performance too much.
    http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/index.htm

    6. a nice comparison of different implementations of rotations:
    http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
    Currently the rotation can be initialized from either:
    XYZ Euler angles
    Rotation Matrix
    Quaternion
*/
class IMPALGEBRAEXPORT Rotation3D {
public:
  //! Create an invalid rotation
  Rotation3D():a_(0.0),b_(0.0),c_(0.0),d_(0.0) {}
  //! Create a rotation from a quaternion
  /** \throw ValueException if the rotation is not a rotation matrix.
   */
  Rotation3D(double a, double b, double c, double d){
    IMP_check(std::abs(square(a)+square(b)+square(c)+square(d) - 1.0) < .05,
              "Attempting to construct a rotation from a non-quaternion value."
              << " The coefficient vector must have a length of 1. Got: "
              << a << " " << b << " " << c << " " << d,
              ValueException);
    a_=a;b_=b;c_=c;d_=d;
    if (a<0) {
      // make them canonical
      a_=-a_;
      b_=-b_;
      c_=-c_;
      d_=-d_;
    }
  }
  ~Rotation3D();
  //! Rotate a vector around the origin
  Vector3D rotate(const Vector3D &o) const {
    IMP_check(a_ != 0 || b_ != 0 || c_ != 0 || d_ != 0,
              "Attempting to apply uninitialized rotation",
              InvalidStateException);
    return Vector3D((a_*a_+b_*b_-c_*c_-d_*d_)*o[0] +
                         2*(b_*c_-a_*d_)*o[1] + 2*(b_*d_+a_*c_)*o[2],
                     2*(b_*c_+a_*d_)*o[0] +
                         (a_*a_-b_*b_+c_*c_-d_*d_)*o[1] + 2*(c_*d_-a_*b_)*o[2],
                     2*(b_*d_-a_*c_)*o[0] +
                         2*(c_*d_+a_*b_)*o[1] + (a_*a_-b_*b_-c_*c_+d_*d_)*o[2]);
  }
  void show(std::ostream& out = std::cout) const {
    out <<a_<<"|"<<b_<<"|"<<c_<<"|"<<d_<<'\n';
  }
  //! Return the rotation which undoes this rotation.
  Rotation3D get_inverse() const;

  //! return the quaterion so that it can be stored
  VectorD<4> get_quaternion() const {
    return VectorD<4>(a_, b_, c_, d_);
  }
private:
  double a_,b_,c_,d_;
};

IMP_OUTPUT_OPERATOR(Rotation3D)


//! Return a rotation that does not do anything
/** \relates Rotation3D */
inline Rotation3D identity_rotation() {
  return Rotation3D(1,0,0,0);
}

//! Initialize a rotation in x-y-z order from three angles
/** \param[in] xr Rotation around the X axis in radians
    \param[in] yr Rotation around the Y axis in radians
    \param[in] zr Rotation around the Z axis in radians
    \note The three rotations are represented in the original (fixed)
    coordinate frame. http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    \relates Rotation3D
*/
inline Rotation3D rotation_from_fixed_xyz(double xr,double yr, double zr)
{
  double a,b,c,d;
  double cx = cos(xr);  double cy = cos(yr);  double cz = cos(zr);
  double sx = sin(xr);  double sy = sin(yr);  double sz = sin(zr);
  double m00 = cz*cy;
  double m11 = -sy*sx*sz + cx*cz;
  double m22 = cy*cx;
  Float zero = 0.0;
  a = sqrt(std::max(1+m00+m11+m22,zero))/2.0;
  b = sqrt(std::max(1+m00-m11-m22,zero))/2.0;
  c = sqrt(std::max(1-m00+m11-m22,zero))/2.0;
  d = sqrt(std::max(1-m00-m11+m22,zero))/2.0;
  if (cy*sx + sy*cx*sz + sx*cz < 0.0) b = -b;
  if (sz*sx - sy*cx*cz - sy < 0.0)    c = -c;
  if (sz*cy + sy*sx*cz + sz*cx < 0.0) d = -d;
  return Rotation3D(a,b,c,d);
}

//! Initialize a rotation from euler angles
/**
    \param[in] phi   Rotation around the Z axis in radians
    \param[in] theta Rotation around the X axis in radians
    \param[in] psi   Rotation around the Z axis in radians
    \note The first rotation is by an angle phi about the z-axis.
          The second rotation is by an angle theta in [0,pi] about the
          former x-axis , and the third rotation is by an angle psi
          about the former z-axis.
    \note http://en.wikipedia.org/wiki/
         Conversion_between_quaternions_and_Euler_angles
    \relates Rotation3D
*/
inline Rotation3D rotation_from_fixed_zxz(double phi, double theta, double psi)
{
  double a,b,c,d;
  double c1,c2,c3,s1,s2,s3;
  c2=cos(theta/2);c1=cos(phi/2);c3=cos(psi/2);
  s2=sin(theta/2);s1=sin(phi/2);s3=sin(psi/2);
  a = c1*c2*c3+s1*s2*s3;
  b = s1*c2*c3-c1*s2*s3;
  c = c1*s2*c3+s1*c2*s3;
  d = c1*c2*s3-s1*s2*c3;
  return Rotation3D(a,b,c,d);
}



//! Generate a Rotation3D object from a rotation matrix
/**
   \throw ValueException if the rotation is not a rotation matrix.
   \relates Rotation3D
 */
IMPALGEBRAEXPORT Rotation3D
rotation_from_matrix(double m11,double m12,double m13,
                     double m21,double m22,double m23,
                     double m31,double m32,double m33);

//! Generate a Rotation3D object from a rotation around an axis
/**
  \param[in] axis the rotation axis
  \param[in] angle the rotation angle in radians
  \note http://en.wikipedia.org/wiki/Rotation_matrix
  \note http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/index.htm
*/
inline Rotation3D rotation_about_axis(Vector3D axis, Float angle)
{
  //normalize the vector
  Vector3D axis_norm = axis.get_unit_vector();
  Float s;
  s=sin(angle/2);
  Float x,y,z;
  x=axis_norm[0];
  y=axis_norm[1];
  z=axis_norm[2];
  Float a,b,c,d;
  a = cos(angle/2);
  b = x*s;
  c = y*s;
  d = z*s;
  return Rotation3D(a,b,c,d);
}


//! Pick a rotation at random from all possible rotations
/** \relates Rotation3D */
IMPALGEBRAEXPORT Rotation3D random_rotation();


IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_3D_H */
