/**
 *  \file  Cylinder3D.h
 *  \brief stores a cylinder
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_CYLINDER_3D_H
#define IMPALGEBRA_CYLINDER_3D_H

#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <iostream>
#include <IMP/constants.h>

/** The cylinder is represented by two points that define
the cylinder direction and height. The radius of the cylinder
is set by a third parameter.

todo:
- document everything
- fix overview docs
*/
IMPALGEBRA_BEGIN_NAMESPACE
class IMPALGEBRAEXPORT Cylinder3D
{
 public:
  Cylinder3D(const Vector3D &start,const Vector3D &end, double radius);
  //! Get a transformation from the cylinder reference frame to the base one.
  /**
  The transformation places the cylinder direction on the Z-axis
  */
  Transformation3D get_transformation_to_base_reference_frame() const;
  Vector3D get_center() const {return start_+(end_-start_)*0.5;}
  Vector3D get_direction() const {return (end_-start_).get_unit_vector();}
  double get_radius() const {return radius_;}
  double get_height() const {return (end_-start_).get_magnitude();}
  double get_surface_area() const;
  double get_volume() const;
  //! Return one of the two points that define the cylinder
  /**
    /param[in] the point index (0 or 1).
   */
  Vector3D get_point(int i) const;
 protected:
  Vector3D start_;
  Vector3D end_;
  double radius_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CYLINDER_3D_H */
