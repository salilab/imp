/**
 *  \file Plane3D.cpp   \brief Simple 3D plane class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Plane3D.h>
IMPALGEBRA_BEGIN_NAMESPACE
Plane3D::Plane3D(const Vector3D& point_on_plane,
                 const Vector3D &normal_to_plane) {
  normal_ = normal_to_plane.get_unit_vector();
  distance_= normal_*point_on_plane;
}
Plane3D::Plane3D(double distance,
                 const Vector3D &normal):
  distance_(distance),
  normal_(normal){
  IMP_check(std::abs(normal.get_squared_magnitude()-1) < .05,
            "The normal vector must be normalized",
            ValueException);
  }

Vector3D Plane3D::get_projection(const Vector3D &p) const {
  return p-normal_*(normal_*p-distance_);
}
bool Plane3D::get_is_above(const Vector3D &p) const {
  return normal_*p > distance_;
}
bool Plane3D::get_is_below(const Vector3D &p) const {
  return normal_*p < distance_;
}

void Plane3D::show(std::ostream &out) const {
    out << "(" << distance_ << ": " << spaces_io(normal_)
        << ")";
}

IMPALGEBRA_END_NAMESPACE
