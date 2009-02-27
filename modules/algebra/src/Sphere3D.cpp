/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/vector_generators.h>
IMPALGEBRA_BEGIN_NAMESPACE
Sphere3D::Sphere3D(Vector3D center,Float radius):center_(center),
                                                 radius_(radius){
}
Float Sphere3D::get_volume() const {
  return PI * (4.0f / 3.0f) * powf(radius_, 3.0f);
}

Float Sphere3D::get_surface_area() const {
  return PI * 4.0f * powf(radius_, 2.0f);
}

IMPALGEBRA_END_NAMESPACE
