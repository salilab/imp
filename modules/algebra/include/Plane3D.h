/**
 *  \file IMP/algebra/Plane3D.h   \brief Simple 3D plane class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_PLANE_3D_H
#define IMPALGEBRA_PLANE_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a plane in 3D.
   \geometry
*/
class Plane3D : public GeometricPrimitiveD<3> {
 public:
  Plane3D() {}
  Plane3D(const Vector3D &point_on_plane, const Vector3D &normal_to_plane)
      : normal_(normal_to_plane) {
    distance_ = normal_ * point_on_plane;
    IMP_USAGE_CHECK_FLOAT_EQUAL(normal_.get_squared_magnitude(), 1,
                                "The normal vector must be normalized");
  }
  Plane3D(double distance_to_plane, const Vector3D &normal_to_plane)
      : distance_(distance_to_plane), normal_(normal_to_plane) {
    IMP_USAGE_CHECK_FLOAT_EQUAL(normal_.get_squared_magnitude(), 1,
                                "The normal vector must be normalized");
  }
  Vector3D get_point_on_plane() const { return normal_ * distance_; }
  const Vector3D &get_normal() const { return normal_; }
  //! Project the point onto the plane
  Vector3D get_projected(const Vector3D &p) const {
    return p - normal_ * (normal_ * p - distance_);
  }
#ifndef IMP_DOXYGEN
  Vector3D get_projection(const Vector3D &p) const { return get_projected(p); }
#endif
  /** @name Orientation
       Up is the direction of the normal. You really shouldn't use
       these as they aren't very reliable.
       @{
  */
  bool get_is_above(const Vector3D &p) const { return get_height(p) > 0; }
  bool get_is_below(const Vector3D &p) const { return get_height(p) < 0; }
  /** @} */
  double get_height(const Vector3D &p) const { return normal_ * p - distance_; }
  IMP_SHOWABLE_INLINE(Plane3D, {
    out << "(" << distance_ << ": " << spaces_io(normal_) << ")";
  });

  //! Return the plane with the opposite normal
  Plane3D get_opposite() const { return Plane3D(-distance_, -normal_); }
  double get_distance_from_origin() const { return distance_; }

 private:
  double distance_;
  Vector3D normal_;  //normal to plane
};

//! Return the distance between a plane and a point in 3D
/** See Plane3D */
inline double get_distance(const Plane3D &pln, const Vector3D &p) {
  return (pln.get_projection(p) - p).get_magnitude();
}

//! return the point reflected about the plane
inline Vector3D get_reflected(const Plane3D &pln, const Vector3D &p) {
  Vector3D proj = pln.get_projected(p);
  return p + 2 * (proj - p);
}

IMP_AREA_GEOMETRY_METHODS(Plane3D, plane_3d, IMP_UNUSED(g);
                          return std::numeric_limits<double>::infinity(), {
  IMP_UNUSED(g);
  Vector3D ip = get_ones_vector_d<3>(std::numeric_limits<double>::infinity());
  return BoundingBoxD<3>(ip) + BoundingBox3D(-ip);
});
IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_PLANE_3D_H */
