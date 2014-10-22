/**
 *  \file  Cone3D.cpp
 *  \brief simple implementation of cones in 3D
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Cone3D.h>
IMPALGEBRA_BEGIN_NAMESPACE

Cone3D::Cone3D(const Segment3D &s, double radius) : seg_(s), radius_(radius) {}
bool Cone3D::get_contains(const Vector3D &v) const {
  Vector3D d = (v - get_tip()).get_unit_vector();
  double x = std::acos(d * get_direction().get_unit_vector());
  return (x >= 0) && (x < (get_angle() / 2)) &&
         (get_distance(v, get_tip()) < get_height());
}

Plane3D Cone3D::get_base_plane() const {
  return Plane3D(seg_.get_point(1), -seg_.get_direction());
}

IMPALGEBRA_END_NAMESPACE
