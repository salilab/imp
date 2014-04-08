/**
 *  \file IMP/algebra/Cylinder3D.h
 *  \brief stores a cylinder
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_CYLINDER_3D_H
#define IMPALGEBRA_CYLINDER_3D_H

#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include "Segment3D.h"
#include "GeometricPrimitiveD.h"
#include <IMP/base/showable_macros.h>
#include <iostream>
#include "constants.h"

IMPALGEBRA_BEGIN_NAMESPACE
/** It does what is says.

    \geometry
*/
class IMPALGEBRAEXPORT Cylinder3D : public GeometricPrimitiveD<3> {
 public:
  Cylinder3D() {}
  Cylinder3D(const Segment3D& s, double radius);
  double get_radius() const { return radius_; }
  const Segment3D& get_segment() const { return s_; }

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
  const Vector3D get_surface_point_at(double relative_height, double angle);

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
      @param angle angle in radians about the cylinder axis, with 0 set to an
                   arbitrary but consistent direction
   */
  const Vector3D get_inner_point_at(double relative_height,
                                    double relative_radius, double angle) const;

  IMP_SHOWABLE_INLINE(Cylinder3D, { out << s_ << ": " << radius_; });

 private:
  Segment3D s_;
  double radius_;
};

IMP_VOLUME_GEOMETRY_METHODS(Cylinder3D, cylinder_3d,
                            return 2.0 * PI * g.get_radius() *
                                       g.get_segment().get_length() +
                                   2.0 * PI * get_squared(g.get_radius()),
                            return PI * get_squared(g.get_radius()) *
                                   g.get_segment().get_length(),
                            IMP_UNUSED(g);
                            IMP_NOT_IMPLEMENTED);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CYLINDER_3D_H */
