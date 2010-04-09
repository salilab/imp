/**
 *  \file Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ROTATION_3D_H
#define IMPALGEBRA_ROTATION_3D_H

#include "algebra_config.h"
#include "Vector3D.h"
#include "utility.h"
#include <IMP/constants.h>

#include <IMP/log.h>
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
class IMPALGEBRAEXPORT Rotation3D {
  VectorD<4> v_;
  mutable bool has_cache_;
  mutable VectorD<3> matrix_[3];
  IMP_NO_SWIG(friend Rotation3D compose(const Rotation3D &a,
                                        const Rotation3D &b));
  void fill_cache() const {
    if (has_cache_) return;
    has_cache_=true;
    matrix_[0]= VectorD<3>(v_[0]*v_[0]+v_[1]*v_[1]-v_[2]*v_[2]-v_[3]*v_[3],
                           2*(v_[1]*v_[2]-v_[0]*v_[3]),
                           2*(v_[1]*v_[3]+v_[0]*v_[2]));
    matrix_[1]= VectorD<3>(2*(v_[1]*v_[2]+v_[0]*v_[3]),
                           v_[0]*v_[0]-v_[1]*v_[1]+v_[2]*v_[2]-v_[3]*v_[3],
                           2*(v_[2]*v_[3]-v_[0]*v_[1]));
    matrix_[2]= VectorD<3>(2*(v_[1]*v_[3]-v_[0]*v_[2]),
                           2*(v_[2]*v_[3]+v_[0]*v_[1]),
                           v_[0]*v_[0]-v_[1]*v_[1]-v_[2]*v_[2]+v_[3]*v_[3]);
  }
 public:
  //! Create a rotation from an unnormalized vector 4
  Rotation3D(const VectorD<4> &v): v_(v.get_unit_vector()){}

  //! Create an invalid rotation
  Rotation3D():v_(0,0,0,0) {}
  //! Create a rotation from a quaternion
  /** \throw ValueException if the rotation is not a unit vector.
   */
  Rotation3D(double a, double b, double c, double d): v_(a,b,c,d),
    has_cache_(false) {
    IMP_USAGE_CHECK(std::abs(v_.get_squared_magnitude() - 1.0) < .1,
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
  VectorD<3> get_rotated_no_cache(const VectorD<3> &o) const {
    return VectorD<3>((v_[0]*v_[0]+v_[1]*v_[1]-v_[2]*v_[2]-v_[3]*v_[3])*o[0]
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
  double get_rotated_one_coordinate_no_cache(const VectorD<3> &o,
                                             unsigned int coord) const {
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
                IndexException);
    }
  }
#endif
  //! Rotate a vector around the origin
  VectorD<3> get_rotated(const VectorD<3> &o) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    fill_cache();
    return VectorD<3>(o*matrix_[0],
                      o*matrix_[1],
                      o*matrix_[2]);
  }

  //! Gets only the requested rotation coordinate of the vector
  double get_rotated_one_coordinate(const VectorD<3> &o,
                                    unsigned int coord) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
                    "Attempting to apply uninitialized rotation");
    fill_cache();
    return o*matrix_[coord];
  }

  //! Rotate a vector around the origin
  VectorD<3> operator*(const VectorD<3> &v) const {
    return get_rotated(v);
  }

  IMP_SHOWABLE_INLINE({out << v_[0] << " " << v_[1]<< " " <<v_[2]
                           << " " <<v_[3];});

  //! Return the rotation which undoes this rotation.
  inline Rotation3D get_inverse() const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() != 0,
                    "Attempting to invert uninitialized rotation");
    Rotation3D ret(v_[0], -v_[1], -v_[2], -v_[3]);
    return ret;
  }


  //! return the quaterion so that it can be stored
  /** Note that there is no guarantee on which of the two
      equivalent quaternions is returned.
  */
  const VectorD<4>& get_quaternion() const {
    return v_;
  }

  //! multiply two rotations
  Rotation3D operator*(const Rotation3D& q) const {
    return compose(*this, q);
  }

  //! Compute the rotation which when composed with r gives this
  Rotation3D operator/(const Rotation3D &r) const {
    return compose(*this, r.get_inverse());
  }

  const Rotation3D &operator/=(const Rotation3D &r) {
    *this= *this/r;
    return *this;
  }

  /** \brief Return the derivative of the position x with respect to
      internal variable i. */
  const VectorD<3> get_derivative(const VectorD<3> &o, unsigned int i) const;
};


IMP_OUTPUT_OPERATOR(Rotation3D);

IMP_VALUES(Rotation3D, Rotation3Ds);


//! Return a rotation that does not do anything
/** \relatesalso Rotation3D */
inline Rotation3D get_identity_rotation_3d() {
  return Rotation3D(1,0,0,0);
}

//! Return a distance between the two rotations
/** The distance runs between 0 and 1. More precisely,
    the distance returned is the angle from the origin
    of the two quaternion vectors (with signs chosen
    appropriately), divided by pi/2.
    \relatesalso Rotation3D
*/
inline double get_distance(const Rotation3D &r0,
                           const Rotation3D &r1) {
  double dot= std::abs(r0.get_quaternion()*r1.get_quaternion());
  if (dot >1) dot=1;
  if (dot < -1) dot=-1;
  double theta= std::acos(dot);
  return 2.0*theta/PI;
}

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
Rotation3D get_rotation_about_axis(const VectorD<3>& axis,
                                   double angle);

//! Create a rotation from the first vector to the second one.
/** \relatesalso Rotation3D
 */
IMPALGEBRAEXPORT
Rotation3D get_rotation_taking_first_to_second(const VectorD<3> &v1,
                                               const VectorD<3> &v2);

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
   \ingroup uninitialized_default
*/
class FixedZYZ {
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
  IMP_SHOWABLE_INLINE({out << v_[0] << " " << v_[1]
                           << " " << v_[2];});
};



//! A simple class for returning XYZ Euler angles
/**
   \ingroup uninitialized_default
*/
class FixedXYZ {
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
  IMP_SHOWABLE_INLINE({
      out << v_[0] << " " << v_[1] << " " << v_[2];
    });
};


IMP_OUTPUT_OPERATOR(FixedZYZ);

//! The inverse of rotation_from_fixed_zyz()
/**
   \see rotation_from_fixed_zyz()
   \relatesalso Rotation3D
   \relatesalso FixedZYZ
*/
IMPALGEBRAEXPORT FixedZYZ get_fixed_zyz_from_rotation(const Rotation3D &r);


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

/** Return the rotation which takes the x and y axes to the given x and y axes.
    The two axis must be perpendicular unit vectors.
*/
IMPALGEBRAEXPORT
Rotation3D get_rotation_from_x_y_axes(const VectorD<3> &x,
                                      const VectorD<3> &y);

//! Decompose a Rotation3D object into a rotation around an axis
/**
   \note http://en.wikipedia.org/wiki/Rotation_matrix
   \note www.euclideanspace.com/maths/geometry/rotations/conversions/
   angleToQuaternion/index.htm
   \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT
std::pair<VectorD<3>,double> get_axis_and_angle(const Rotation3D &rot);

typedef std::pair<VectorD<3>,double> AxisAnglePair;
#ifndef IMP_DOXYGEN
typedef std::vector<AxisAnglePair> AxisAnglePairs;
#endif

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_3D_H */
