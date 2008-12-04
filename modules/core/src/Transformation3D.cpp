/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/core/Transformation3D.h"
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

Transformation3D::~Transformation3D(){}


IMPCOREEXPORT Transformation3D
transformation_from_rotation_around_vector(const Rotation3D &rot,
                                           const Vector3D &center) {
  Vector3D rc= rot.rotate(center);
  return Transformation3D(rot, -rc + center);
}


IMPCORE_END_NAMESPACE
