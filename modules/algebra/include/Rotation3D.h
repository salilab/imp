/**
 *  \file IMP/algebra/Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ROTATION_3D_H
#define IMPALGEBRA_ROTATION_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "utility.h"
#include "constants.h"
#include "GeometricPrimitiveD.h"

#include <IMP/base/log.h>
#include <cmath>
#include <iostream>
#include <algorithm>

IMPALGEBRA_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class Rotation3D;
Rotation3D compose(const Rotation3D &a, const Rotation3D &b) ;
#endif


//! 3D rotation class.
/** Rotations are currently represented using quaternions and a cached
    copy of the rotation matrix. The quaternion allows for fast and
    stable composition and the cached rotation matrix means that
    rotations are performed quickly. See
    http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation for
    a comparison of different implementations of rotations.

    Currently the rotation can be initialized from either:
    - XYZ Euler angles
    - Rotation Matrix
    - Quaternion
    - angle/axis representation

    \geometry
*/
class IMPALGEBRAEXPORT Rotation3D: public GeometricPrimitiveD<3> {
  VectorD<4> v_;
  mutable bool has_cache_;
  mutable Vector3D matrix_[3];
  IMP_NO_SWIG(friend Rotation3D compose(const Rotation3D &a,
                                        const Rotation3D &b));
  void fill_cache() const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    has_cache_=true;
    double v0s=get_squared(v_[0]);
    double v1s=get_squared(v_[1]);
    double v2s=get_squared(v_[2]);
    double v3s=get_squared(v_[3]);
    double v12= v_[1]*v_[2];
    double v01=v_[0]*v_[1];
    double v02=v_[0]*v_[2];
    double v23=v_[2]*v_[3];
    double v03=v_[0]*v_[3];
    double v13=v_[1]*v_[3];
    matrix_[0]= Vector3D(v0s+v1s-v2s-v3s,
                           2*(v12-v03),
                           2*(v13+v02));
    matrix_[1]= Vector3D(2*(v12+v03),
                           v0s-v1s+v2s-v3s,
                           2*(v23-v01));
    matrix_[2]= Vector3D(2*(v13-v02),
                           2*(v23+v01),
                           v0s-v1s-v2s+v3s);
  }
 public:
  //! Create a rotation from an unnormalized vector 4
  Rotation3D(const VectorD<4> &v):
    v_(v.get_unit_vector()),
    has_cache_(false){}

  //! Create an invalid rotation
  Rotation3D():v_(0,0,0,0), has_cache_(false) {}
  //! Create a rotation from a quaternion
  /** \throw base::ValueException if the rotation is not a unit vector.
   */
  Rotation3D(double a, double b, double c, double d): v_(a,b,c,d),
    has_cache_(false) {
    IMP_USAGE_CHECK_FLOAT_EQUAL(v_.get_squared_magnitude(), 1.0,
                    "Attempting to construct a rotation from a "
                    << " non-quaternion value. The coefficient vector"
                    << " must have a length of 1. Got: "
                    << a << " " << b << " " << c << " " << d
                    << " gives " << v_.get_squared_magnitude());
    if (a<0) {
      // make them canonical
      v_=-v_;
    }

  }
  ~Rotation3D();


#ifndef IMP_DOXYGEN
  Vector3D get_rotated_no_cache(const Vector3D &o) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to access uninitialized rotation");
    return Vector3D((v_[0]*v_[0]+v_[1]*v_[1]-v_[2]*v_[2]-v_[3]*v_[3])*o[0]
                      + 2*(v_[1]*v_[2]-v_[0]*v_[3])*o[1]
                      + 2*(v_[1]*v_[3]+v_[0]*v_[2])*o[2],
                      2*(v_[1]*v_[2]+v_[0]*v_[3])*o[0]
                      + (v_[0]*v_[0]-v_[1]*v_[1]+v_[2]*v_[2]-v_[3]*v_[3])*o[1]
                      + 2*(v_[2]*v_[3]-v_[0]*v_[1])*o[2],
                      2*(v_[1]*v_[3]-v_[0]*v_[2])*o[0]
                      + 2*(v_[2]*v_[3]+v_[0]*v_[1])*o[1]
                      + (v_[0]*v_[0]-v_[1]*v_[1]-v_[2]*v_[2]+v_[3]*v_[3])*o[2]);
  }

  //! Gets only the requested rotation coordinate of the vector
  double get_rotated_one_coordinate_no_cache(const Vector3D &o,
                                             unsigned int coord) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    switch(coord) {
    case 0:
      return (v_[0]*v_[0]+v_[1]*v_[1]-v_[2]*v_[2]-v_[3]*v_[3])*o[0]
        + 2*(v_[1]*v_[2]-v_[0]*v_[3])*o[1]
        + 2*(v_[1]*v_[3]+v_[0]*v_[2])*o[2];
      break;
    case 1:
      return 2*(v_[1]*v_[2]+v_[0]*v_[3])*o[0]
        + (v_[0]*v_[0]-v_[1]*v_[1]+v_[2]*v_[2]-v_[3]*v_[3])*o[1]
        + 2*(v_[2]*v_[3]-v_[0]*v_[1])*o[2];

      break;
    case 2:
      return 2*(v_[1]*v_[3]-v_[0]*v_[2])*o[0]
        + 2*(v_[2]*v_[3]+v_[0]*v_[1])*o[1]
        + (v_[0]*v_[0]-v_[1]*v_[1]-v_[2]*v_[2]+v_[3]*v_[3])*o[2];
      break;
    default:
      IMP_THROW("Out of range coordinate " << coord,
                base::IndexException);
    }
  }
#endif
  //! Rotate a vector around the origin
  Vector3D get_rotated(const Vector3D &o) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    if (!has_cache_) fill_cache();
    return Vector3D(o*matrix_[0],
                      o*matrix_[1],
                      o*matrix_[2]);
  }

  //! Gets only the requested rotation coordinate of the vector
  double get_rotated_one_coordinate(const Vector3D &o,
                                    unsigned int coord) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    if (!has_cache_) fill_cache();
    return o*matrix_[coord];
  }

  //! Rotate a vector around the origin
  Vector3D operator*(const Vector3D &v) const {
    return get_rotated(v);
  }
  Vector3D get_rotation_matrix_row(int i) const {
    IMP_USAGE_CHECK((i>=0)&&(i<=2),"row index out of range");
    if (!has_cache_) fill_cache();
    return matrix_[i];
  }
  IMP_SHOWABLE_INLINE(Rotation3D, {out << v_[0] << " " << v_[1]<< " " <<v_[2]
                           << " " <<v_[3];});

  //! Return the rotation which undoes this rotation.
  inline Rotation3D get_inverse() const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() > 0,
                    "Attempting to invert uninitialized rotation");
    Rotation3D ret(v_[0], -v_[1], -v_[2], -v_[3]);
    return ret;
  }


  //! Return the quaternion so that it can be stored
  /** Note that there is no guarantee on which of the two
      equivalent quaternions is returned.
  */
  const Vector4D& get_quaternion() const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to access uninitialized rotation");
    return v_;
  }
  //! multiply two rotations
  Rotation3D operator*(const Rotation3D& q) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to compose uninitialized rotation");
    return compose(*this, q);
  }

  //! Compute the rotation which when composed with r gives this
  Rotation3D operator/(const Rotation3D &r) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to compose uninitialized rotation");
    return compose(*this, r.get_inverse());
  }

  const Rotation3D &operator/=(const Rotation3D &r) {
    *this= *this/r;
    return *this;
  }

  /** \brief Return the derivative of the position x with respect to
      internal variable i. */
  const Vector3D get_derivative(const Vector3D &o, unsigned int i) const;
};

IMP_VALUES(Rotation3D, Rotation3Ds);


//! Return a rotation that does not do anything
/** \relatesalso Rotation3D */
inline Rotation3D get_identity_rotation_3d() {
  return Rotation3D(1,0,0,0);
}

//! Return a distance between the two rotations
/** The distance runs between 0 and 1. More precisely,
    the distance returned is distance between the two
    quaternion vectors properly normalized, divided
    by sqrt(2).

    A vector with distance d from the unit vector
    represents a rotation of

    \f$ \theta= \cos^{-1}\left(1-\sqrt{2}d\right)\f$
    \relatesalso Rotation3D
*/
inline double get_distance(const Rotation3D &r0,
                           const Rotation3D &r1) {
  double dot= (r0.get_quaternion()-r1.get_quaternion()).get_squared_magnitude();
  double odot= (r0.get_quaternion()
                +r1.get_quaternion()).get_squared_magnitude();
  double ans= std::min(dot, odot);
  const double s2=std::sqrt(2.0);
  double ret= ans/s2;
  return std::max(std::min(ret, 1.0), 0.0);
}


//! Generate a Rotation3D object from a rotation around an axis
//! that is assumed to be normalized
/**
   \param[in] axis_norm the normalized rotation axis passing through (0,0,0)
   \param[in] angle the rotation angle in radians in the
   clockwise direction
   \note http://en.wikipedia.org/wiki/Rotation_matrix
   \note www.euclideanspace.com/maths/geometry/rotations/conversions/
   angleToQuaternion/index.htm
   \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT
Rotation3D get_rotation_about_normalized_axis
(const Vector3D& axis_norm,
 double angle);


//! Generate a Rotation3D object from a rotation around an axis
/**
   \param[in] axis the rotation axis passes through (0,0,0)
   \param[in] angle the rotation angle in radians in the
   clockwise direction
   \note http://en.wikipedia.org/wiki/Rotation_matrix
   \note www.euclideanspace.com/maths/geometry/rotations/conversions/
   angleToQuaternion/index.htm
   \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT
Rotation3D get_rotation_about_axis(const Vector3D& axis,
                                   double angle);

//! Create a rotation from the first vector to the second one.
/** \relatesalso Rotation3D
 */
IMPALGEBRAEXPORT
Rotation3D get_rotation_taking_first_to_second(const Vector3D &v1,
                                               const Vector3D &v2);

//! Generate a Rotation3D object from a rotation matrix
/**
   \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT Rotation3D
get_rotation_from_matrix(double m00,double m01,double m02,
                         double m10,double m11,double m12,
                         double m20,double m21,double m22);




//! Pick a rotation at random from all possible rotations
/** \relatesalso Rotation3D */
IMPALGEBRAEXPORT Rotation3D get_random_rotation_3d();


//! Pick a rotation at random near the provided one
/** This method generates a rotation that is within the provided
    distance of center.
    \param[in] center The center of the rotational volume
    \param[in] distance See
    get_distance(const Rotation3D&,const Rotation3D&)
    for a full definition.

    \note The cost of this operation increases as distance goes to 0.

    \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT Rotation3D get_random_rotation_3d(const Rotation3D &center,
                                                   double distance);


//! Cover the space of rotations evenly
/** If you care about the distance between samples instead of the number
    of samples, the "surface area" of the set of rotations is pi^2. If
    you allocate each sample a volume of 4/3 pi d^3 (to space them d apart),
    Then you want 3/4 pi/d^3 points.

    Creates at least num_points rotations.
*/
IMPALGEBRAEXPORT Rotation3Ds
get_uniform_cover_rotations_3d(unsigned int num_points);

//! Generates a nondegenerate set of Euler angles with a delta resolution
/**
\param[in] delta sample every delta angles in radians.
 */
IMPALGEBRAEXPORT
algebra::Rotation3Ds get_uniformly_sampled_rotations(double delta);

//! Compute a rotatation from an unnormalized quaternion
/** \relatesalso Rotation3D */
inline Rotation3D get_rotation_from_vector4d(const VectorD<4> &v) {
  VectorD<4> uv= v.get_unit_vector();
  return Rotation3D(uv[0], uv[1], uv[2], uv[3]);
}


/** \relatesalso Rotation3D
 */
inline Rotation3D compose(const Rotation3D &a, const Rotation3D &b) {
  return Rotation3D(a.v_[0]*b.v_[0] - a.v_[1]*b.v_[1]
                    - a.v_[2]*b.v_[2] - a.v_[3]*b.v_[3],
                    a.v_[0]*b.v_[1] + a.v_[1]*b.v_[0]
                    + a.v_[2]*b.v_[3] - a.v_[3]*b.v_[2],
                    a.v_[0]*b.v_[2] - a.v_[1]*b.v_[3]
                    + a.v_[2]*b.v_[0] + a.v_[3]*b.v_[1],
                    a.v_[0]*b.v_[3] + a.v_[1]*b.v_[2]
                    - a.v_[2]*b.v_[1] + a.v_[3]*b.v_[0]);
}

/** \name Euler Angles
    There are many conventions for how to define Euler angles, based on choices
    of which of the x,y,z axis to use in what order and whether the rotation
    axis is in the body frame (and hence affected by previous rotations) or in
    in a fixed frame. See
    http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    for a general description.

    - All Euler angles are specified in radians.
    - The names are all \c rotation_from_{fixed/body}_abc() where abc is the
    ordering of x,y,z.
    @{
*/

//! Initialize a rotation in x-y-z order from three angles
/** \param[in] xr Rotation around the X axis in radians
    \param[in] yr Rotation around the Y axis in radians
    \param[in] zr Rotation around the Z axis in radians
    \note The three rotations are represented in the original (fixed)
    coordinate frame.
    \relatesalso Rotation3D
    \relatesalso FixedXYZ
*/
IMPALGEBRAEXPORT Rotation3D get_rotation_from_fixed_xyz(double xr,
                                                        double yr,
                                                        double zr);

//! Initialize a rotation from euler angles
/**
   \param[in] phi   Rotation around the Z axis in radians
   \param[in] theta Rotation around the X axis in radians
   \param[in] psi   Rotation around the Z axis in radians
   \note The first rotation is by an angle phi about the z-axis.
   The second rotation is by an angle theta in [0,pi] about the
   former x-axis , and the third rotation is by an angle psi
   about the former z-axis.
   \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT Rotation3D get_rotation_from_fixed_zxz(double phi,
                                                        double theta,
                                                        double psi);

//! Generate a rotation object from Euler Angles
/** \note The first rotation is by an angle about the z-axis.
    The second rotation is by an angle about the new y-axis.
    The third rotation is by an angle about the new z-axis.
    \param[in] Rot First Euler angle (radians) defining the rotation (Z axis)
    \param[in] Tilt Second Euler angle (radians) defining the rotation (Y axis)
    \param[in] Psi Third Euler angle (radians) defining the rotation (Z axis)
    \relatesalso Rotation3D
    \relatesalso FixedZYZ
*/
IMPALGEBRAEXPORT Rotation3D get_rotation_from_fixed_zyz(double Rot,
                                                        double Tilt,
                                                        double Psi);



//! A simple class for returning ZYZ Euler angles
/**
*/
class FixedZYZ: public GeometricPrimitiveD<3> {
  double v_[3];
public:
  FixedZYZ(){}
  FixedZYZ(double rot, double tilt, double psi)
  {v_[0]=rot; v_[1]= tilt; v_[2]=psi;}
  double get_rot() const {
    return v_[0];
  }
  double get_tilt() const {
    return v_[1];
  }
  double get_psi() const {
    return v_[2];
  }
  IMP_SHOWABLE_INLINE(FixedZYZ,
                      {out << v_[0] << " " << v_[1]
                           << " " << v_[2];});
};

IMP_VALUES(FixedZYZ, FixedZYZs);




//! A simple class for returning ZXZ Euler angles
class FixedZXZ: public GeometricPrimitiveD<3> {
  double v_[3];
public:
  FixedZXZ(){}
  FixedZXZ(double psi, double theta, double phi)
  {v_[0]=psi; v_[1]= theta; v_[2]=phi;}
  double get_psi() const {
    return v_[0];
  }
  double get_theta() const {
    return v_[1];
  }
  double get_phi() const {
    return v_[2];
  }
  IMP_SHOWABLE_INLINE(FixedZXZ,
                      {out << v_[0] << " " << v_[1]
                           << " " << v_[2];});
};

IMP_VALUES(FixedZXZ, FixedZXZs);


//! A simple class for returning XYZ Euler angles
class FixedXYZ: public GeometricPrimitiveD<3>  {
  double v_[3];
public:
  FixedXYZ(){}
  FixedXYZ(double x, double y, double z)
  {v_[0]=x; v_[1]= y; v_[2]=z;}
  double get_x() const {
    return v_[0];
  }
  double get_y() const {
    return v_[1];
  }
  double get_z() const {
    return v_[2];
  }
  IMP_SHOWABLE_INLINE(FixedXYZ, {
      out << v_[0] << " " << v_[1] << " " << v_[2];
    });
};

IMP_VALUES(FixedXYZ, FixedXYZs);

//! The inverse of rotation_from_fixed_zyz()
/**
   \see rotation_from_fixed_zyz()
   \relatesalso Rotation3D
   \relatesalso FixedZYZ
*/
IMPALGEBRAEXPORT FixedZYZ get_fixed_zyz_from_rotation(const Rotation3D &r);


//! The inverse of rotation_from_fixed_zyz()
/**
   \see rotation_from_fixed_zxz()
   \relatesalso Rotation3D
   \relatesalso FixedZXZ
*/
IMPALGEBRAEXPORT FixedZXZ get_fixed_zxz_from_rotation(const Rotation3D &r);

//! The inverse of rotation_from_fixed_xyz()
/**
   \see rotation_from_fixed_xyz()
   \relatesalso Rotation3D
   \relatesalso FixesXYZ
*/
IMPALGEBRAEXPORT FixedXYZ get_fixed_xyz_from_rotation(const Rotation3D &r);

/** @}*/


//! Interpolate between two rotations
/** It f ==0, return b, if f==1 return a.
    \relatesalso Rotation3D*/
inline Rotation3D get_interpolated(const Rotation3D &a,
                                   const Rotation3D &b,
                                   double f) {
  VectorD<4> bq= b.get_quaternion(), aq= a.get_quaternion();
  if (bq*aq < 0) bq=-bq;
  return f*aq+(1-f)*bq;
}

/** Return the rotation which takes the native x and y axes to the
    given x and y axes.
    The two axis must be perpendicular unit vectors.
*/
IMPALGEBRAEXPORT
Rotation3D get_rotation_from_x_y_axes(const Vector3D &x,
                                      const Vector3D &y);

//! Decompose a Rotation3D object into a rotation around an axis
/**
   \note http://en.wikipedia.org/wiki/Rotation_matrix
   \note www.euclideanspace.com/maths/geometry/rotations/conversions/
   angleToQuaternion/index.htm
   \relatesalso Rotation3D

   @return axis direction and rotation about the axis in Radians
*/
IMPALGEBRAEXPORT
std::pair<Vector3D,double> get_axis_and_angle(const Rotation3D &rot);



typedef std::pair<Vector3D,double> AxisAnglePair;
#ifndef IMP_DOXYGEN
typedef base::Vector<AxisAnglePair> AxisAnglePairs;
#endif

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_3D_H */
