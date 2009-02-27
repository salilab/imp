/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Sphere3D.h>
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE
Sphere3D::Sphere3D(const Vector3D& center,double radius):center_(center),
                                                 radius_(radius){
}
Float Sphere3D::get_volume() const {
  return PI * (4.0 / 3.0) * std::pow(radius_, 3.0);
}

Float Sphere3D::get_surface_area() const {
  return PI * 4.0 * square(radius_);
}

IMPALGEBRA_END_NAMESPACE
