/**
 *  \file Rotation3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ROTATION_3D_H
#define IMPALGEBRA_ROTATION_3D_H

#include "config.h"
#include "Vector3D.h"

#include <IMP/log.h>

#include <iostream>
#include <algorithm>

IMPALGEBRA_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class Rotation3D;
Rotation3D compose(const Rotation3D &a, const Rotation3D &b) ;
#endif


#ifndef IMP_DOXYGEN
/* According to benchmarks/rotate having the cache is 20% faster. Since space
   is unlikely to be an issue for rotations, we can keep the cache. */
#define IMP_ROTATION_CACHE
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
#ifdef IMP_ROTATION_CACHE
  mutable bool has_cache_;
  mutable Vector3D matrix_[3];
#endif
  IMP_NO_SWIG(friend Rotation3D compose(const Rotation3D &a,
                                        const Rotation3D &b));
  void fill_cache() const {
#ifdef IMP_ROTATION_CACHE
    if (has_cache_) return;
    has_cache_=true;
    matrix_[0]= Vector3D(v_[0]*v_[0]+v_[1]*v_[1]-v_[2]*v_[2]-v_[3]*v_[3],
                         2*(v_[1]*v_[2]-v_[0]*v_[3]),
                         2*(v_[1]*v_[3]+v_[0]*v_[2]));
    matrix_[1]= Vector3D(2*(v_[1]*v_[2]+v_[0]*v_[3]),
                         v_[0]*v_[0]-v_[1]*v_[1]+v_[2]*v_[2]-v_[3]*v_[3],
                         2*(v_[2]*v_[3]-v_[0]*v_[1]));
    matrix_[2]= Vector3D(2*(v_[1]*v_[3]-v_[0]*v_[2]),
                         2*(v_[2]*v_[3]+v_[0]*v_[1]),
                         v_[0]*v_[0]-v_[1]*v_[1]-v_[2]*v_[2]+v_[3]*v_[3]);
#endif
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
                    << " gives " << v_.get_squared_magnitude(),
              ValueException);
    if (a<0) {
      // make them canonical
      v_=-v_;
    }

  }
  ~Rotation3D();


#ifndef IMP_DOXYGEN
  Vector3D rotate_no_cache(const Vector3D &o) const {
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
  double rotate_one_coordinate_no_cache(const Vector3D &o,
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
    }
  }
#endif

  //! Rotate a vector around the origin
  Vector3D rotate(const Vector3D &o) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
              "Attempting to apply uninitialized rotation",
              InvalidStateException);
#ifdef IMP_ROTATION_CACHE
    fill_cache();
    return Vector3D(o*matrix_[0],
                    o*matrix_[1],
                    o*matrix_[2]);
#else
    return rotate_no_cache(o);
#endif
  }

  //! Gets only the requested rotation coordinate of the vector
  double rotate_one_coordinate(const Vector3D &o,unsigned int coord) const {
    IMP_USAGE_CHECK(v_.get_squared_magnitude() >0,
              "Attempting to apply uninitialized rotation",
              InvalidStateException);
#ifdef IMP_ROTATION_CACHE
    fill_cache();
    return o*matrix_[coord];
#else
    return rotate_one_coordinate_no_cache(o,coord);
#endif
  }

  //! Rotate a vector around the origin
  Vector3D operator*(const Vector3D &v) const {
    return rotate(v);
  }

  IMP_SHOWABLE_INLINE({out << v_[0] << " " << v_[1]<< " " <<v_[2]
                           << " " <<v_[3];})

  //! Return the rotation which undoes this rotation.
  Rotation3D get_inverse() const;

  //! return the quaterion so that it can be stored
  const VectorD<4>& get_quaternion() const {
    return v_;
  }

  //! return the quaterion so that it can be stored
  /** This quaternion has it sign chosen so as to be interoperable
      with q (that is, they are in the same hemisphere). Use this
      when writing code to average or clustering rotations.*/
  const VectorD<4> get_quaternion(const VectorD<4> &q) const {
    if (v_*q < 0) return -v_;
    else return v_;
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
  const Vector3D get_derivative(const Vector3D &o, unsigned int i) const {
    /* The computation was derived in maple. Source code is probably in
       modules/algebra/tools
     */
    double t4 = v_[0]*o[0] - v_[3]*o[1] + v_[2]*o[2];
    double t5 = square(v_[0]);
    double t6 = square(v_[1]);
    double t7 = square(v_[2]);
    double t8 = square(v_[3]);
    double t9 = t5 + t6 + t7 + t8;
    double t10 = 1.0/t9;
    double t11 = 2*t4*t10;
    double t14 = v_[1]*v_[2];
    double t15 = v_[0]*v_[3];

    double t19 = v_[1]*v_[3];
    double t20 = v_[0]*v_[2];
    double t25 = square(t9);
    double t26 = 1.0/t25;

    double t27 = ((t5 + t6 - t7 - t8)*o[0] + 2*(t14 - t15)*o[1]
                  + 2*(t19 + t20)*o[2])*t26;

    double t34 = v_[3]*o[0] + v_[0]*o[1] - v_[1]*o[2];
    double t35 = 2*t34*t10;
    double t41 = v_[2]*v_[3];
    double t42 = v_[0]*v_[1];

    double t47 = (2*(t14 + t15)*o[0] + (t5 - t6 + t7 - t8)*o[1]
                  + 2*(t41 - t42)*o[2])*t26;

    double t54 = -v_[2]*o[0] + v_[1]*o[1] + v_[0]*o[2];
    double t55 = 2*t54*t10;

    double t65 = (2*(t19 - t20)*o[0] + 2*(t41 + t42)*o[1]
                  + (t5 - t6 - t7 + t8)*o[2])*t26;

    double t73 = 2*(v_[1]*o[0] + v_[2]*o[1] + v_[3]*o[2])*t10;

    /*all[1, 1] = t11 - 2*t27*v_[0];
      all[1, 2] = t35 - 2*t47*v_[0];
      all[1, 3] = t55 - 2*t65*v_[0];

      all[2, 1] = t73 - 2*t27*v_[1];
      all[2, 2] = -2*t54 t10 - 2*t47*v_[1];
      all[2, 3] = t35 - 2*t65*v_[1];

      all[3, 1] = t55 - 2*t27*v_[2];
      all[3, 2] = t73 - 2*t47*v_[2];
      all[3, 3] = -2*t4 t10 - 2*t65*v_[2];

      all[4, 1] = -2*t34 t10 - 2*t27*v_[3];
      all[4, 2] = t11 - 2*t47*v_[3];
      all[4, 3] = t73 - 2*t65*v_[3];
    */

    switch (i) {
    case 0:
    return Vector3D(t11 - 2*t27*v_[0],
                    t35 - 2*t47*v_[0],
                    t55 - 2*t65*v_[0]);
    case 1:
    return Vector3D(t73 - 2*t27*v_[1],
                    -2*t54*t10 - 2*t47*v_[1],
                    t35 - 2*t65*v_[1]);
    case 2:
    return Vector3D(t55 - 2*t27*v_[2],
                    t73 - 2*t47*v_[2],
                    -2*t4*t10 - 2*t65*v_[2]);
    case 3:
    return Vector3D(-2*t34*t10 - 2*t27*v_[3],
                    t11 - 2*t47*v_[3],
                    t73 - 2*t65*v_[3]);
    default:
      throw IndexException("Invalid derivative component");
    };
    return Vector3D(0,0,0);
  }
};


IMP_OUTPUT_OPERATOR(Rotation3D)

typedef std::vector<Rotation3D> Rotation3Ds;


//! Return a rotation that does not do anything
/** \relatesalso Rotation3D */
inline Rotation3D identity_rotation() {
  return Rotation3D(1,0,0,0);
}

//! Generate a Rotation3D object from a rotation around an axis
/**
  \param[in] axis the rotation axis passes through (0,0,0)
  \param[in] angle the rotation angle in radians
  \note http://en.wikipedia.org/wiki/Rotation_matrix
  \note www.euclideanspace.com/maths/geometry/rotations/conversions/
  angleToQuaternion/index.htm
  \relatesalso Rotation3D
*/
inline Rotation3D rotation_in_radians_about_axis(const Vector3D& axis,
                                                 double angle)
{
  //normalize the vector
  Vector3D axis_norm = axis.get_unit_vector();
  double s = std::sin(angle/2);
  double a,b,c,d;
  a = std::cos(angle/2);
  b = axis_norm[0]*s;
  c = axis_norm[1]*s;
  d = axis_norm[2]*s;
  return Rotation3D(a,b,c,d);
}

//! Create a rotation from the first vector to the second one.
/** \relatesalso Rotation3D
 */
inline Rotation3D rotation_taking_first_to_second(const Vector3D &v1,
                                                  const Vector3D &v2) {
    Vector3D v1_norm = v1.get_unit_vector();
    Vector3D v2_norm = v2.get_unit_vector();
    //get a vector that is perpendicular to the plane containing v1 and v2
    Vector3D vv = vector_product(v1_norm,v2_norm);
    //get the angle between v1 and v2
    double dot = v1_norm*v2_norm;
    dot = ( dot < -1.0 ? -1.0 : ( dot > 1.0 ? 1.0 : dot ) );
    double angle = std::acos(dot);
    //check a special case: the input vectors are parallel / antiparallel
    if (std::abs(dot) == 1.0) {
      IMP_LOG(VERBOSE," the input vectors are (anti)parallel "<<std::endl);
      return rotation_in_radians_about_axis(orthogonal_vector(v1),angle);
    }
    return rotation_in_radians_about_axis(vv,angle);
}

//! Generate a Rotation3D object from a rotation matrix
/**
   \relatesalso Rotation3D
 */
IMPALGEBRAEXPORT Rotation3D
rotation_from_matrix(double m00,double m01,double m02,
                     double m10,double m11,double m12,
                     double m20,double m21,double m22);




//! Pick a rotation at random from all possible rotations
/** \relatesalso Rotation3D */
IMPALGEBRAEXPORT Rotation3D random_rotation();


//! Pick a rotation at random near the provided one
/** This method generates a rotation that is within the provided
    distance of center.
    \param[in] center The center of the rotational volume
    \param[in] distance A distance of 0 means only center can be
    returned, and a distance of 1 means all rotations can be returned.
    The exact metric used is the distance between the
    quaternions divided by two.

    \note The cost of this operation increases as distance goes to 0.

    \relatesalso Rotation3D
*/
IMPALGEBRAEXPORT Rotation3D random_rotation(const Rotation3D &center,
                                            double distance);


//! Cover the space of rotations evenly
/** If you care about the distance between samples instead of the number
    of samples, the "surface area" of the set of rotations is pi^2. If
    you allocate each sample a volume of 4/3 pi d^3 (to space them d apart),
    Then you want 3/4 pi/d^3 points.

    Creates at least num_points rotations.
*/
IMPALGEBRAEXPORT Rotation3Ds uniform_cover_rotations(unsigned int num_points);

//! Compute a rotatation from an unnormalized quaternion
/** \relatesalso Rotation3D */
inline Rotation3D rotation_from_vector4d(const VectorD<4> &v) {
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
IMPALGEBRAEXPORT Rotation3D rotation_from_fixed_xyz(double xr,
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
IMPALGEBRAEXPORT Rotation3D rotation_from_fixed_zxz(double phi,
                                                    double theta,
                                                    double psi);

//! Generate a rotation object from Euler Angles
/**    \note The first rotation is by an angle about the z-axis.
          The second rotation is by an angle about the new y-axis.
          The third rotation is by an angle about the new z-axis.
    \param[in] Rot First Euler angle (radians) defining the rotation (Z axis)
    \param[in] Tilt Second Euler angle (radians) defining the rotation (Y axis)
    \param[in] Psi Third Euler angle (radians) defining the rotation (Z axis)
    \relatesalso Rotation3D
    \relatesalso FixedZYZ
*/
IMPALGEBRAEXPORT Rotation3D rotation_from_fixed_zyz(double Rot,
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
                           << " " << v_[2];})
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
    })
};


IMP_OUTPUT_OPERATOR(FixedZYZ);

//! The inverse of rotation_from_fixed_zyz()
/**
   \see rotation_from_fixed_zyz()
   \relatesalso Rotation3D
   \relatesalso FixedZYZ
 */
IMPALGEBRAEXPORT FixedZYZ fixed_zyz_from_rotation(const Rotation3D &r);


//! The inverse of rotation_from_fixed_xyz()
/**
   \see rotation_from_fixed_xyz()
   \relatesalso Rotation3D
   \relatesalso FixesXYZ
 */
IMPALGEBRAEXPORT FixedXYZ fixed_xyz_from_rotation(const Rotation3D &r);

/** @}*/


//! Interpolate between two rotations
/** It f ==0, return b, if f==1 return a.
    \relatesalso Rotation3D*/
inline Rotation3D interpolate(const Rotation3D &a,
                              const Rotation3D &b,
                              double f) {
  return f*a.get_quaternion()+(1-f)*b.get_quaternion(a.get_quaternion());
}


//! Decompose a Rotation3D object into a rotation around an axis
/**
  \note http://en.wikipedia.org/wiki/Rotation_matrix
  \note www.euclideanspace.com/maths/geometry/rotations/conversions/
  angleToQuaternion/index.htm
  \relatesalso Rotation3D
*/
inline std::pair<Vector3D,double> decompose_rotation_into_axis_angle(
  const Rotation3D &rot) {
  VectorD<4> q = rot.get_quaternion();
  double a,b,c,d;
  a=q[0];b=q[1];c=q[2];d=q[3];

  double angle = std::acos(a)*2;
  double s = std::sin(angle/2);
  Vector3D axis(b/s,c/s,d/s);
  return std::pair<Vector3D,double>(axis.get_unit_vector(),angle);
}



IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_3D_H */
