/**
 *  \file  Cylinder3D.cpp
 *  \brief stores a cylinder
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Cylinder3D.h>

IMPALGEBRA_BEGIN_NAMESPACE
Cylinder3D::Cylinder3D(const Segment3D &s,
                       double radius)
  :s_(s),radius_(radius){
}

Transformation3D Cylinder3D::get_transformation_to_place_direction_on_Z() const{
  Vector3D main_dir = get_segment().get_direction();
  Vector3D vertical_dir = orthogonal_vector(main_dir);
  Transformation3D move2zero= Transformation3D(
                   identity_rotation(),-get_segment().get_middle_point());
  //transformation_from_reference_frame(a,b,c) , sets the Z-axis to
  //be prependicular to a and b. We want Z to be the main direction of
  //the cylinder
  Transformation3D rigid_trans=transformation_from_reference_frame(
                                 vertical_dir,
                                 vector_product(main_dir,vertical_dir),
                                 Vector3D(0.0,0.0,0.0));
  return rigid_trans.get_inverse();
}

double Cylinder3D::get_surface_area() const {
  return 2.0*PI*radius_ * get_segment().get_length() +
    2.0*PI *radius_*radius_;
}
double Cylinder3D::get_volume() const {
  return PI *radius_*radius_ * get_segment().get_length();
}

IMPALGEBRA_END_NAMESPACE
