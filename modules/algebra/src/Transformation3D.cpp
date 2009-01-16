/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation3D::~Transformation3D(){}


IMPALGEBRAEXPORT Transformation3D
transformation_from_rotation_around_vector(const Rotation3D &rot,
                                           const Vector3D &center) {
  Vector3D rc= rot.rotate(center);
  return Transformation3D(rot, -rc + center);
}


IMPALGEBRA_END_NAMESPACE
