/**
 *  \file Transformation2D.cpp
 *  \brief Simple 2D transformation class.
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation2D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation2D::~Transformation2D(){}

Transformation2D Transformation2D::get_inverse() const{
  Rotation2D inv_rot = rot_.get_inverse();
  return Transformation2D(inv_rot,-(inv_rot.rotate(trans_)));
}

IMPALGEBRA_END_NAMESPACE
