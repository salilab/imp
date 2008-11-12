/**
 *  \file Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_ROTATION_3D_H
#define IMPMISC_ROTATION_3D_H

#include "misc_exports.h"

#include "Matrix3D.h"
IMPMISC_BEGIN_NAMESPACE

//! 3D rotation class.
/** \ingroup helper
*/

class IMPMISCEXPORT Rotation3D {
public:
  Rotation3D(){
 }
  //! Initialize a rotation in x-y-z order from three Euler angles
  /** \param[in] xr Rotation around the X axis
      \param[in] yr Rotation around the Y axis
      \param[in] zr Rotation around the Z axis
  */
  Rotation3D(Float xr, Float yr, Float zr) {
    init_angles(xr,yr,zr);
  }
  //! Initialize a rotation in x-y-z order from three identical Euler angles
  /** \param[in] e_angle Rotation around first the X axis, Y axis and Z axis
  */
  Rotation3D(Float e_angle){
    init_angles(e_angle, e_angle, e_angle);
  }
  //! Initialize a rotation in x-y-z order from three Euler angles
  /** \param[in] v  A vector that holds three Euler angles (x-y-z order)
  */
  Rotation3D(const Vector3D &v){
    init_angles(v[0],v[1],v[2]);
  }
  Matrix3D get_matrix() const {
  const Float a = quat_[0];
  const Float b = quat_[1];
  const Float c = quat_[2];
  const Float d = quat_[3];
  return Matrix3D(a*a+b*b-c*c-d*d, 2*(b*c-a*d)    , 2*(b*d+a*c),
                  2*(b*c+a*d)    , a*a-b*b+c*c-d*d, 2*(c*d-a*b),
                  2*(b*d-a*c)    , 2*(c*d+a*b)    , a*a-b*b-c*c+d*d);
  }
  //! Rotation by vector multiplication
  Vector3D operator*(const Vector3D &o) const {
  const Float a = quat_[0];
  const Float b = quat_[1];
  const Float c = quat_[2];
  const Float d = quat_[3];
  return Vector3D((a*a+b*b-c*c-d*d)*o[0] + 2*(b*c-a*d)*o[1] + 2*(b*d+a*c)*o[2],
                  2*(b*c+a*d)*o[0] + (a*a-b*b+c*c-d*d)*o[1] + 2*(c*d-a*b)*o[2],
                  2*(b*d-a*c)*o[0] + 2*(c*d+a*b)*o[1] + (a*a-b*b-c*c+d*d)*o[2]);
  }
  //! Returns the rotation around the x-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  Float rotX() const{
    return atan2(matrix32(), matrix33());
  }

  //! Returns the rotation around the y-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  Float rotY() const{
    return atan2(matrix31(), sqrt(sqr(matrix21())+sqr(matrix11())));
  }

  //! Returns the rotation around the z-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  Float rotZ() const{
    return atan2(matrix21(), matrix11());
  }

private:
  void init_angles(Float xr, Float yr, Float zr) {
    Float cx = cos(xr);  Float cy = cos(yr);  Float cz = cos(zr);
    Float sx = sin(xr);  Float sy = sin(yr);  Float sz = sin(zr);
    Float m00 = cz*cy;
    Float m11 = -sy*sx*sz + cx*cz;
    Float m22 = cy*cx;
    quat_[0] = sqrt(1+m00+m11+m22)/2.0;
    quat_[1] = sqrt(1+m00-m11-m22)/2.0;
    quat_[2] = sqrt(1-m00+m11-m22)/2.0;
    quat_[3] = sqrt(1-m00-m11+m22)/2.0;
    if (cy*sx + sy*cx*sz + sx*cz < 0.0) quat_[1] = -quat_[1];
    if (sz*sx - sy*cx*cz - sy < 0.0)    quat_[2] = -quat_[2];
    if (sz*cy + sy*sx*cz + sz*cx < 0.0) quat_[3] = -quat_[3];
  }

  Float matrix11() const {
    return sqr(quat_[0]) + sqr(quat_[1]) - sqr(quat_[2]) - sqr(quat_[3]);
  }
  Float matrix12() const {
    return 2*(quat_[1]*quat_[2] - quat_[0]*quat_[3]);
  }
  Float matrix13()  const{
    return 2*(quat_[2]*quat_[3] + quat_[0]*quat_[2]);
  }
  Float matrix21() const {
    return 2*(quat_[1]*quat_[2] + quat_[0]*quat_[3]);
  }
  Float matrix22() const {
    return sqr(quat_[0]) - sqr(quat_[1]) + sqr(quat_[2]) - sqr(quat_[3]);
  }
  Float matrix23() const {
    return 2*(quat_[2]*quat_[3] - quat_[0]*quat_[1]);
  }
  Float matrix31() const {
    return 2*(quat_[1]*quat_[3] - quat_[0]*quat_[2]);
  }
  Float matrix32() const {
    return 2*(quat_[2]*quat_[3] + quat_[0]*quat_[1]);
  }
  Float matrix33() const {
    return sqr(quat_[0]) - sqr(quat_[1]) - sqr(quat_[2]) + sqr(quat_[3]);
  }

private:
  Float sqr(Float a)const{return a*a;}
  Float quat_[4];
};
IMPMISC_END_NAMESPACE
#endif  /* IMPMISC_ROTATION_3D_H */
