/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation3D::~Transformation3D(){}
Transformation3D Transformation3D::get_inverse() const{
  Rotation3D inv_rot = rot_.get_inverse();
  return Transformation3D(inv_rot,-(inv_rot.rotate(trans_)));
}

Transformation3D build_Transformation3D_from_Transformation2D(
                                  const Transformation2D &t2d) {
  Rotation3D R = rotation_from_fixed_zyz(
                          t2d.get_rotation().get_angle(),0.0,0.0);
  Vector3D t(t2d.get_translation()[0],t2d.get_translation()[1],0);
  return Transformation3D(R,t);
}

IMPALGEBRA_END_NAMESPACE
