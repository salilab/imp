/**
 *  \file  Cylinder3D.h
 *  \brief stores a cylinder
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_CYLINDER_3D_H
#define IMPALGEBRA_CYLINDER_3D_H

#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include "Segment3D.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/** The cylinder is represented by two points that define
    the cylinder direction and height. The radius of the cylinder
    is set by a third parameter.

    todo:
    - document everything
    - fix overview docs
*/
class IMPALGEBRAEXPORT Cylinder3D: public UninitializedDefault
{
 public:
  Cylinder3D(const Segment3D &s, double radius);
  //! Get a transformation that places the cylinder direction on Z
  /**
  /note the function would work if the center of the reference frame is (0,0,0)
  */
  Transformation3D get_transformation_to_place_direction_on_Z() const;
  double get_radius() const {return radius_;}
  //! Get surface area, including the caps
  double get_surface_area() const;
  double get_volume() const;
  const Segment3D& get_segment() const {
    return s_;
  }
 private:
  Segment3D s_;
  double radius_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CYLINDER_3D_H */
