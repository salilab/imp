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
#include <cmath>


/** We represent a cylinder by :
       center point
       main direction
       radius size
       length
this is a right cylinder

todo:
- get_point(int)
- remove _get{high,low}*
- doc whether area includes caps
- remove get_bild_string
- get_main_direction->get_direction
- remove PI
- document everything
- fix overview docs
*/
IMPALGEBRA_BEGIN_NAMESPACE
const double PI = 4.0*std::atan(1.0);
class IMPALGEBRAEXPORT Cylinder3D
{
 public:
  Cylinder3D(const Vector3D &start,const Vector3D &end, Float radius);
  Transformation3D get_transformation_to_base_reference_frame() const;
  Vector3D get_center() const {return start_+(end_-start_)*0.5;}
  Vector3D get_main_direction() const {return (end_-start_).get_unit_vector();}
  Float get_radius() const {return radius_;}
  Float get_height() const {return (end_-start_).get_magnitude();}
  std::string get_bild_string() const;
  Float get_area() const;
  Float get_volume() const;
  Vector3D get_low_base_point() const {return start_;}
  Vector3D get_high_base_point() const {return start_;}
 protected:
  Vector3D start_;
  Vector3D end_;
  Float radius_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CYLINDER_3D_H */
