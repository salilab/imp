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
#include "IMP/algebra/Rotation3D.h"

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
  Transformation3D(const Rotation3D& r, const Vector3D& t)
    : trans_(t), rot_(r){}
  ~Transformation3D();
  //! transform
  Vector3D transform(const Vector3D &o) const {
    return rot_.rotate(o) + trans_;
  }
  //! get the rotation part
  const Rotation3D& get_rotation() const {
    return rot_;
  }
  //! Get the translation part
  const Vector3D& get_translation()const{return trans_;}

  void show(std::ostream& out = std::cout) const {
    rot_.show(out);
    out<<" || "<<trans_<<"\n";
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

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_TRANSFORMATION_3D_H */
