/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation3D::~Transformation3D(){}
Transformation3D Transformation3D::get_inverse() const{
  Rotation3D inv_rot = rot_.get_inverse();
  return Transformation3D(inv_rot,-(inv_rot.rotate(trans_)));
}

Transformation3D Transformation3D::multiply(const Transformation3D &trans2) {
  /*return Transformation3D(rot_.multiply(trans2.rot_),
    transform(trans2.trans_));*/
}

IMPALGEBRAEXPORT Transformation3D
transformation_from_rotation_around_vector(const Rotation3D &rot,
                                           const Vector3D &center) {
  Vector3D rc= rot.rotate(center);
  return Transformation3D(rot, -rc + center);
}


IMPALGEBRA_END_NAMESPACE
