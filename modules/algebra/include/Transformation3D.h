/**
 *  \file Transformation3D.h   \brief Simple 3D transformation class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_TRANSFORMATION_3D_H
#define IMPALGEBRA_TRANSFORMATION_3D_H

#include "algebra_config.h"
#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation2D.h"

IMPALGEBRA_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class Transformation3D;
Transformation3D compose(const Transformation3D &a,
                         const Transformation3D &b);
#endif

//! Simple 3D transformation class
/** The rotation is applied first, and then the point is translated.
    \see IMP::core::Transform
    \geometry
*/
class IMPALGEBRAEXPORT Transformation3D
{
public:
  // public for swig
  IMP_NO_DOXYGEN(typedef Transformation3D This);
  //! construct an invalid transformation
  Transformation3D(){}
  /** basic constructor*/
  Transformation3D(const Rotation3D& r,
                   const VectorD<3>& t=VectorD<3>(0,0,0)):
    trans_(t), rot_(r){}
  /** Construct a transformation with an identity rotation.*/
  Transformation3D(const VectorD<3>& t):
    trans_(t), rot_(get_identity_rotation_3d()){}
  ~Transformation3D();
  //! transform
  VectorD<3> get_transformed(const VectorD<3> &o) const {
    return rot_.get_rotated(o) + trans_;
  }
  //! apply transformation (rotate and then translate)
  VectorD<3> operator*(const VectorD<3> &v) const {
    return get_transformed(v);
  }
  /** compose two rigid transformation such that for any vector v
      (rt1*rt2)*v = rt1*(rt2*v) */
  Transformation3D operator*(const Transformation3D &tr) const {
    return compose(*this, tr);
  }
  const Transformation3D& operator*=(const Transformation3D &o) {
    *this=compose(*this, o);
    return *this;
  }
  /** Compute the transformation which, when composed with b, gives *this.
      That is a(x)== d(b(x)) for all x.

      For consistency, this should probably have a nice name, but
      I don't know what name to give it.
  */
  Transformation3D operator/(const Transformation3D &b) const {
    Transformation3D ret= compose(*this, b.get_inverse());
    return ret;
  }
  const Transformation3D& operator/=(const Transformation3D &o) {
    *this= *this/o;
    return *this;
  }
  const Rotation3D& get_rotation() const {
    return rot_;
  }
  const VectorD<3>& get_translation()const{return trans_;}

  IMP_SHOWABLE_INLINE(Transformation3D, {
      rot_.show(out);
      out<<" || "<<trans_;
    }
    );
  Transformation3D get_inverse() const;
  //! Check if the transformation is valid
  bool is_valid() const {
    return rot_.is_valid();
  }
private:
  VectorD<3> trans_; //tranlation
  Rotation3D rot_;  //rotation
};

IMP_OUTPUT_OPERATOR(Transformation3D);
IMP_VALUES(Transformation3D, Transformation3Ds);


//! Return a transformation that does not do anything
/** \relatesalso Transformation3D */
inline Transformation3D get_identity_transformation_3d() {
  return Transformation3D(get_identity_rotation_3d(),VectorD<3>(0.0,0.0,0.0));
}

//! Generate a Transformation3D object from a rotation around a point
/** Rotate about a point rather than the origin.
  \param[in] point Center to rotate about
  \param[in] rotation The rotation to perform

  \relatesalso Transformation3D
*/
inline Transformation3D
get_rotation_about_point(const VectorD<3> &point,
                     const Rotation3D &rotation) {
  return Transformation3D(rotation, (rotation*(-point)+point));
}

//! compose two transformations
  /** For any vector v (a*b)*v = a*(b*v).
      \relatesalso Transformation3D
   */
inline Transformation3D compose(const Transformation3D &a,
                                const Transformation3D &b){
  return Transformation3D(compose(a.get_rotation(), b.get_rotation()),
                          a.get_transformed(b.get_translation()));
}


//! Builds a 3D transformation from a 2D one.
/**
  \note The 3D transformation is built with the 2D rotation becoming a rotation
  around the z axis.
  **/
IMPALGEBRAEXPORT Transformation3D get_transformation_3d(
                                  const Transformation2D &t2d);

//! Get a local transformation
/**
  \note randomly select an axis that passes to the input point
  and rotate around it
  \param[in] origin the origin of the rotation
  \param[in] max_translation detault value is 5
  \param[in] max_angle_in_rad default value is 15 degree in radians
  **/
IMPALGEBRAEXPORT Transformation3D get_random_local_transformation(
   Vector3D origin,
   float max_translation=5.,
   float max_angle_in_rad=0.26);


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_TRANSFORMATION_3D_H */
