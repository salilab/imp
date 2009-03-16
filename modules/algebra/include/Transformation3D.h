/**
 *  \file Transformation3D.h   \brief Simple 3D transformation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_TRANSFORMATION_3D_H
#define IMPALGEBRA_TRANSFORMATION_3D_H

#include "config.h"

#include "Vector3D.h"
#include "Rotation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Simple 3D transformation class
/**
*/
class IMPALGEBRAEXPORT Transformation3D
{
public:
  // public for swig
  typedef Transformation3D This;
  //! construct and invalid transformation
  Transformation3D(){}
  /** basic constructor*/
  Transformation3D(const Rotation3D& r,
                   const Vector3D& t=Vector3D(0,0,0)):
    trans_(t), rot_(r){}
  /** Construct a transformation with an identity rotation.*/
  Transformation3D(const Vector3D& t):
    trans_(t), rot_(identity_rotation()){}
  ~Transformation3D();
  //! transform
  Vector3D transform(const Vector3D &o) const {
    return rot_.rotate(o) + trans_;
  }
  //! apply transformation
  Vector3D operator*(const Vector3D &v) {
    return transform(v);
  }
  /** multiply two rigid transformation such that for any vector v
      (rt1*rt2)*v = rt1*(rt2*v) */
  Transformation3D operator*(const Transformation3D &tr) {
    return Transformation3D(rot_ * tr.rot_, transform(tr.trans_));
  }
  //! get the rotation part
  const Rotation3D& get_rotation() const {
    return rot_;
  }
  //! Get the translation part
  const Vector3D& get_translation()const{return trans_;}
  //! Compose two transformations
  /**
     /param[in] trans2 The other transformation
     /note for any vector v (trans1*trans2)*v = trans1*(trans2*v).
           Here trans1 is the transformation stored in this.
   */
  Transformation3D compose(const Transformation3D &trans2);
  void show(std::ostream& out = std::cout) const {
    rot_.show(out);
    out<<" || "<<trans_;
  }
  Transformation3D get_inverse() const;
private:
  Vector3D trans_; //tranlation
  Rotation3D rot_;  //rotation
};

IMP_OUTPUT_OPERATOR(Transformation3D)

//! Compute the transformation resulting from rotating around some point
/** This computes the Transformation3D that results when you rotate space
    around the specified point.

    The transformation is rot(x-center)+center
    \relates Transformation3D
 */

IMPALGEBRAEXPORT Transformation3D
transformation_from_rotation_around_vector(const Rotation3D &rot,
                                           const Vector3D &center);
//! Return a transformation that does not do anything
/** \relates Transformation3D */
inline Transformation3D identity_transformation() {
  return Transformation3D(identity_rotation(),Vector3D(0.0,0.0,0.0));
}

//! Generate a transformation from the natural reference-frame to
//  a different one
/**
  \param[in] u     vector used to define the new reference frame
  \param[in] w     vector used to define the new reference frame
  \param[in] base  the center of the new reference frame
  \brief A rotation from the natural reference frame to one defined by u,w
         and base.
         The x-axis lies on u-base.
         The y-axis is perpendicular to both x and z.
         The z-axis is perpendicular to the u-base , w-base plane
 */
inline Transformation3D transformation_from_reference_frame(const Vector3D &u,
                                                const Vector3D &w,
                                                const Vector3D &base) {
  Vector3D x = (u-base);
  Vector3D z = vector_product(x,w-base);
  Vector3D y = vector_product(z,x);
  Vector3D xu = x.get_unit_vector();
  Vector3D zu = z.get_unit_vector();
  Vector3D yu = y.get_unit_vector();
  Rotation3D rot = rotation_from_matrix(xu[0],xu[1],xu[2],
                                        yu[0],yu[1],yu[2],
                                        zu[0],zu[1],zu[2]).get_inverse();
  return Transformation3D(rot,base);
}

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_TRANSFORMATION_3D_H */
