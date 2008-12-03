/**
 *  \file Transformation3D.h   \brief Simple 3D transformation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_TRANSFORMATION_3D_H
#define IMPCORE_TRANSFORMATION_3D_H

#include "core_exports.h"

#include "IMP/Vector3D.h"
#include "IMP/core/Rotation3D.h"

IMPCORE_BEGIN_NAMESPACE

//! Simple 3D transformation class
/**
*/
class IMPCOREEXPORT Transformation3D
{
public:
  // public for swig
  typedef Transformation3D This;
  //! construct and invalid transformation
  Transformation3D(){
  }
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
  //  Matrix3D get_mat()const{return rot_;}
  void show(std::ostream& out = std::cout) const {
    rot_.show(out);
    out<<" || "<<trans_<<"\n";
  }
private:
  Vector3D trans_; //tranlation
  Rotation3D rot_;  //rotation
};

IMP_OUTPUT_OPERATOR(Transformation3D)

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRANSFORMATION_3D_H */
