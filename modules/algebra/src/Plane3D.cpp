/**
 *  \file Plane3D.cpp   \brief Simple 3D plane class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Plane3D.h>
IMPALGEBRA_BEGIN_NAMESPACE
Plane3D::Plane3D(const Vector3D& point_on_plane,
                 const Vector3D &normal_to_plane) {
  point_ = point_on_plane;
  normal_ = normal_to_plane.get_unit_vector();
}
double Plane3D::projection(const Vector3D &p) const {
  return normal_*(p-point_);
}
bool Plane3D::is_above(const Vector3D &p) const {
  return projection(p)>0;
}
bool Plane3D::is_below(const Vector3D &p) const {
  return projection(p)>0;
}

void Plane3D::show(std::ostream &out) const {
    out << "(" << spaces_io(point_) << ": " << spaces_io(normal_)
        << ")";
}

IMPALGEBRA_END_NAMESPACE
