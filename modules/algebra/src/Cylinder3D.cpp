/**
 *  \file  Cylinder3D.cpp
 *  \brief stores a cylinder
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Cylinder3D.h>

IMPALGEBRA_BEGIN_NAMESPACE
Cylinder3D::Cylinder3D(const Vector3D &start,
                       const Vector3D &end,
                       double radius)
           :start_(start),end_(end),radius_(radius){
}
Transformation3D Cylinder3D::get_transformation_to_base_reference_frame() const{
  Vector3D main_dir = get_direction();
  Vector3D vertical_dir = get_vertical_vector(main_dir);
  Transformation3D move2zero= Transformation3D(
                                identity_rotation(),-get_center());
  //transformation_from_reference_frame(a,b,c) , sets the Z-axis to
  //be prependicular to a and b. We want Z to be the main direction of
  //the cylinder
  Transformation3D rigid_trans_inv=transformation_from_reference_frame(
                                     vertical_dir,
                                     vector_product(main_dir,vertical_dir),
                                     Vector3D(0.0,0.0,0.0));
  return rigid_trans_inv.get_inverse().compose(move2zero);
}

double Cylinder3D::get_surface_area() const {
  return 2.0*PI*radius_ * get_height() +
    2.0*PI *radius_*radius_;
}
double Cylinder3D::get_volume() const {
  return PI *radius_*radius_ * get_height();
}
// std::string Cylinder3D::get_bild_string() const {
//   std::ostringstream s;
//   s<<".cylinder " << start_[0] << " " << start_[1] << " " << start_[2]
//    << " " << end_[0] << " "<< end_[1]<<" " <<end_[2]<< " " << radius_
//    << " open "<<std::endl;
//   return s.str();
// }
Vector3D Cylinder3D::get_point(int i) const {
  IMP_assert((i == 1 || i == 0), "Invalid points index");
  if (i==0) {
    return start_;
  }
  return end_;
}

IMPALGEBRA_END_NAMESPACE
