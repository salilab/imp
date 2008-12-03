/**
 *  \file Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_ROTATION_3D_H
#define IMPMISC_ROTATION_3D_H

#include "misc_exports.h"
#include "IMP/Vector3D.h"
#include <iostream>
IMPMISC_BEGIN_NAMESPACE

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

    Currently the rotation can be initialized from either:
    XYZ Euler angles
    Rotation Matrix
    Quaternion
*/

class IMPMISCEXPORT Rotation3D {
public:
  Rotation3D():a_(0.0),b_(0.0),c_(0.0),d_(0.0) {}
  Rotation3D(Float a, Float b, Float c, Float d){
    a_=a;b_=b;c_=c;d_=d;
  }
    //! Rotation by vector multiplication
  Vector3D mult(const Vector3D &o) const {
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
private:
  Float a_,b_,c_,d_;
};


//! Initialize a rotation in x-y-z order from three angles
/** \param[in] xr Rotation around the X axis in radians
      \param[in] yr Rotation around the Y axis in radians
      \param[in] zr Rotation around the Z axis in radians
      \note The three rotations are represented in the original (fixed)
            coordinate frame. http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
*/
inline Rotation3D rotation_from_fixed_xyz(Float xr,Float yr, Float zr)
{
  Float a,b,c,d;
  Float cx = cos(xr);  Float cy = cos(yr);  Float cz = cos(zr);
  Float sx = sin(xr);  Float sy = sin(yr);  Float sz = sin(zr);
  Float m00 = cz*cy;
  Float m11 = -sy*sx*sz + cx*cz;
  Float m22 = cy*cx;
  a = sqrt(1+m00+m11+m22)/2.0;
  b = sqrt(1+m00-m11-m22)/2.0;
  c = sqrt(1-m00+m11-m22)/2.0;
  d = sqrt(1-m00-m11+m22)/2.0;
  if (cy*sx + sy*cx*sz + sx*cz < 0.0) b = -b;
  if (sz*sx - sy*cx*cz - sy < 0.0)    c = -c;
  if (sz*cy + sy*sx*cz + sz*cx < 0.0) d = -d;
  return Rotation3D(a,b,c,d);
}

inline Rotation3D rotation_from_mat(Float m11,Float m12,Float m13,
                                    Float m21,Float m22,Float m23,
                                    Float m31,Float m32,Float m33) {
  Float a,b,c,d;
  a = fabs(1+m11+m22+m33)/4;
  b = fabs(1+m11-m22-m33)/4;
  c = fabs(1-m11+m22-m33)/4;
  d = fabs(1-m11-m22+m33)/4;

  // make sure quat is normalized.
  Float sum = a+b+c+d;
  a = sqrt(a/sum);
  b = sqrt(b/sum);
  c = sqrt(c/sum);
  d = sqrt(d/sum);

  if (m32-m23 < 0.0) b=-b;
  if (m13-m31 < 0.0) c=-c;
  if (m21-m12 < 0.0) d=-d;
  return Rotation3D(a,b,c,d);
}
/*
Rotation3D rotation_from_axis_angle(Vector3D axis, Float a){}
*/
IMPMISC_END_NAMESPACE
#endif  /* IMPMISC_ROTATION_3D_H */
