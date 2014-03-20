/**
 *  \file  Cylinder3D.cpp
 *  \brief stores a cylinder
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Cylinder3D.h>

IMPALGEBRA_BEGIN_NAMESPACE
Cylinder3D::Cylinder3D(const Segment3D &s, double radius)
    : s_(s), radius_(radius) {}

/** Returns a point on the surface of the cylinder,
    specified by its location relative to the cylinder axis
    and its rotation angle about the axis

    @param relative_height a number in the range [0..1] that specifies
                           the point location relative to the cylinder axis
                           such that 0 specifies the cylinder bottom and
                           1 specifies its top
    @param angle angle in radians about the cylinder axis, with 0 set to an
                 arbitrary but consistent direction
*/
const Vector3D Cylinder3D::get_surface_point_at(double relative_height,
                                                double angle) {
  double relative_radius = 1.0;  // surface
  return get_inner_point_at(relative_height, relative_radius, angle);
}

/** Returns a point inside the cylinder,
    specified by its location relative to the cylinder axis,
    its relative radius and  its rotation angle about the axis

    @param relative_height a number in the range [0..1] that specifies
                           the point location relative to the cylinder axis
                           such that 0 specifies the cylinder bottom and
                           1 specifies its top
    @param relative_radius a number in the range [0..1] that specifies
                           the distance of the point from the cylinder axis
                           relative to the cylinder radius, 0 being on the
                           axis itself, and 1 being on the cylinder surface
    @param angle angle in radians about the cylinder axis, with 0 set to
                 an arbitrary but consistent direction
  */
const Vector3D Cylinder3D::get_inner_point_at(double relative_height,
                                              double relative_radius,
                                              double angle) const {
  // compute the point relative to cylinder of equivalent dimensions,
  // but whose main axis segment is z-aligned and lying at the origin
  Vector3D scaling(get_radius() * relative_radius,
                   get_radius() * relative_radius, get_segment().get_length());
  Vector3D pt(scaling[0] * sin(angle), scaling[1] * cos(angle),
              scaling[2] * relative_height);
  // transform the cylinder axis from the origin to the correct location
  Rotation3D rot = get_rotation_taking_first_to_second(
      Vector3D(0, 0, 1),
      get_segment().get_point(1) - get_segment().get_point(0));
  Transformation3D tr(rot, get_segment().get_point(0));
  return tr.get_transformed(pt);
}

IMPALGEBRA_END_NAMESPACE
