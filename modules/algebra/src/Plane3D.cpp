/**
 *  \file Plane3D.cpp   \brief Simple 3D plane class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/internal/cgal_predicates.h>

IMPALGEBRA_BEGIN_NAMESPACE
Plane3D::Plane3D(const VectorD<3>& point_on_plane,
                 const VectorD<3> &normal_to_plane) {
  normal_ = normal_to_plane.get_unit_vector();
  distance_= normal_*point_on_plane;
}
Plane3D::Plane3D(double distance,
                 const VectorD<3> &normal):
  distance_(distance),
  normal_(normal){
  IMP_USAGE_CHECK(std::abs(normal.get_squared_magnitude()-1) < .05,
            "The normal vector must be normalized");
  }

VectorD<3> Plane3D::get_projection(const VectorD<3> &p) const {
  return p-normal_*(normal_*p-distance_);
}
bool Plane3D::get_is_above(const VectorD<3> &p) const {
#ifdef IMP_USE_CGAL
  return internal::cgal_plane_compare_above(*this, p) > 0;
#else
  return normal_*p > distance_;
#endif
}
bool Plane3D::get_is_below(const VectorD<3> &p) const {
#ifdef IMP_USE_CGAL
  return internal::cgal_plane_compare_above(*this, p) < 0;
#else
  return normal_*p < distance_;
#endif
}

void Plane3D::show(std::ostream &out) const {
    out << "(" << distance_ << ": " << spaces_io(normal_)
        << ")";
}

IMPALGEBRA_END_NAMESPACE
