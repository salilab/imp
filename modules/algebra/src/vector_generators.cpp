/**
 *  \file vector_generators.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/algebra/vector_generators.h>


IMPALGEBRA_BEGIN_NAMESPACE

Vector3Ds uniform_cover(const Cylinder3D &cyl,
                        int number_of_points) {
  Vector3Ds points;
  Vector3D starting_point,rotated_point;
  Vector3D z_direction(0.0,0.0,1.0);
  // move the cylinder to the base reference frame (center at (0,0,0)
  // and its main direction to be on the Z axis)
  Transformation3D cyl_rf_to_base_rf =
    cyl.get_transformation_to_place_direction_on_Z();
  Transformation3D move2zero =
              Transformation3D(identity_rotation(),-cyl.get_center());
  for(int i=0;i<number_of_points;i++) {
    ::boost::uniform_real<> rand(0,cyl.get_length());
    starting_point = cyl.get_point(0);
    starting_point = move2zero.transform(starting_point);
    starting_point = cyl_rf_to_base_rf.transform(starting_point);
    starting_point = starting_point +
               Vector3D(cyl.get_radius(),0.0,rand(random_number_generator));
   points.push_back(starting_point);
  }
  for(int i=0;i<number_of_points;i++) {
    ::boost::uniform_real<> rand(0,2*PI);
    //generate a random rotation around the cycle
    Rotation3D rot = rotation_about_axis(z_direction,
                                     rand(random_number_generator));
    rotated_point =  rot.rotate(points[i]);
    //back transformation of the rotated point back to the original cylinder
    rotated_point =  cyl_rf_to_base_rf.get_inverse().transform(rotated_point);
    rotated_point =  move2zero.get_inverse().transform(rotated_point);
    points[i]=rotated_point;
  }
  return points;
}

Vector3Ds grid_cover(const Cylinder3D &cyl,
                     int number_of_cycles, int number_of_points_on_cycle){
  Vector3Ds points;
  // move the cylinder to the base reference frame (center at (0,0,0)
  // and its main direction to be on the Z axis)
  Transformation3D cyl_rf_to_base_rf =
    cyl.get_transformation_to_place_direction_on_Z();
  Vector3D z_direction(0.0,0.0,1.0);
  Float translation_step = cyl.get_length()/number_of_cycles;
  Float rotation_step = 2*PI/number_of_points_on_cycle;
  std::vector<Rotation3D> rotations;
  for(int angle_ind = 0; angle_ind<number_of_points_on_cycle;angle_ind++) {
    rotations.push_back(
         rotation_about_axis(z_direction, angle_ind*rotation_step));
  }
  Vector3D starting_point,rotated_point;
  Transformation3D move2zero =
               Transformation3D(identity_rotation(),-cyl.get_center());
  for(int cycle_ind = 0; cycle_ind<number_of_cycles;cycle_ind++) {
    starting_point = cyl.get_point(0);
    starting_point = move2zero.transform(starting_point);
    starting_point = cyl_rf_to_base_rf.transform(starting_point);
    starting_point = starting_point +
              Vector3D(cyl.get_radius(),0.0,translation_step*cycle_ind);
    for(std::vector<Rotation3D>::iterator i_rot = rotations.begin();
        i_rot != rotations.end();i_rot++) {
      rotated_point =  i_rot->rotate(starting_point);
      //back transformation of the rotated point back to the original cylindar
      rotated_point =  cyl_rf_to_base_rf.get_inverse().transform(rotated_point);
      rotated_point =  move2zero.get_inverse().transform(rotated_point);
      points.push_back(rotated_point);
    }
  }
  return points;
}

Vector3Ds uniform_cover(const Sphere3D &sph,int number_of_points) {
  //find bounding cylinder
  Vector3Ds points;
  //  Cylinder3D cyl = sph.get_bounding_cylinder();
  // Vector3Ds cylinder_sampled_points = uniform_cover(cyl,number_of_points);
  for(int i=0;i<number_of_points;i++){
    points.push_back(
       random_vector_on_sphere(sph.get_center(),sph.get_radius()));
  }
  return points;
}

Vector3Ds uniform_cover(const Sphere3DPatch &sph,
                        unsigned int number_of_points) {
  //find a sphere to sample in
  Vector3D center = sph.get_sphere().get_center()+
  sph.get_sphere().get_radius()*sph.get_plane().get_normal().get_unit_vector();
  double radius = distance(sph.point_on_sphere(),center);
  IMP_check(
   radius > 0.,"The radius should be positive " << radius,ErrorException);
  Vector3Ds points;
  while (points.size() < number_of_points) {
    Vector3D rp = random_vector_in_sphere(center,radius);
    if (sph.get_contains(rp)) {
      points.push_back(rp);
    }
  }
  return points ;
}


IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Cone3D &cone,
                                         unsigned int number_of_points) {
 Vector3Ds points;
 Vector3D sph_p;
 Sphere3D sph = cone.get_bounding_sphere();
 while (points.size() < number_of_points) {
   sph_p=random_vector_in_sphere(sph.get_center(),sph.get_radius());
   if (cone.get_contains(sph_p)) {
     points.push_back(sph_p);
   }
 }
 return points;
}


IMPALGEBRA_END_NAMESPACE
