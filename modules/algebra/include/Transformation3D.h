/**
 *  \file Transformation3D.h   \brief Simple 3D transformation class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_TRANSFORMATION_3D_H
#define IMPALGEBRA_TRANSFORMATION_3D_H

#include "config.h"
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
/** \see IMP::core::Transform
    \geometry
*/
class IMPALGEBRAEXPORT Transformation3D
{
public:
  // public for swig
  IMP_NO_DOXYGEN(typedef Transformation3D This;)
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

  IMP_SHOWABLE_INLINE({
      rot_.show(out);
      out<<" || "<<trans_;
    }
    )
  Transformation3D get_inverse() const;
private:
  VectorD<3> trans_; //tranlation
  Rotation3D rot_;  //rotation
};

IMP_OUTPUT_OPERATOR(Transformation3D)


//! Return a transformation that does not do anything
/** \relatesalso Transformation3D */
inline Transformation3D get_identity_transformation_3d() {
  return Transformation3D(get_identity_rotation_3d(),VectorD<3>(0.0,0.0,0.0));
}

//! Generate a transformation from the natural reference-frame to
//!  a different one
/**
  \param[in] u     vector used to define the new reference frame
  \param[in] w     vector used to define the new reference frame
  \param[in] base  the center of the new reference frame
  \brief A rotation from the natural reference frame to one defined by u,w
         and base.
         The x-axis lies on u-base.
         The y-axis is perpendicular to both x and z.
         The z-axis is perpendicular to the u-base , w-base plane

   \note This function is poorly designed and liable the change. The
   main problem comes from having the three arguments of the same type
   with no natural order amongst them.

   \relatesalso Transformation3D
 */
inline Transformation3D
get_transformation_from_reference_frame(const VectorD<3> &u,
                                        const VectorD<3> &w,
                                        const VectorD<3> &base) {
  VectorD<3> x = (u-base);
  VectorD<3> z = vector_product(x,w-base);
  VectorD<3> y = vector_product(z,x);
  VectorD<3> xu = x.get_unit_vector();
  VectorD<3> zu = z.get_unit_vector();
  VectorD<3> yu = y.get_unit_vector();
  Rotation3D rot = get_rotation_from_matrix(xu[0],xu[1],xu[2],
                                            yu[0],yu[1],yu[2],
                                            zu[0],zu[1],zu[2]).get_inverse();
  return Transformation3D(rot,base);
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
  \note The 3D transformation is built with the angle from the 2D transformation
  as first Euler angle (ZYZ). The other angles are set to 0.
  **/
IMPALGEBRAEXPORT Transformation3D build_Transformation3D_from_Transformation2D(
                                  const Transformation2D &t2d);


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_TRANSFORMATION_3D_H */
