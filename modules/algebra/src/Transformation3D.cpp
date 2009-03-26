/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation3D::~Transformation3D(){}
Transformation3D Transformation3D::get_inverse() const{
  Rotation3D inv_rot = rot_.get_inverse();
  return Transformation3D(inv_rot,-(inv_rot.rotate(trans_)));
}


IMPALGEBRA_END_NAMESPACE
