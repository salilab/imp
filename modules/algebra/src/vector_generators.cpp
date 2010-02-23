/**
 *  \file vector_generators.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/algebra/vector_generators.h>
#include <IMP/log.h>

IMPALGEBRA_BEGIN_NAMESPACE

namespace {
Transformation3D
get_transformation_to_place_direction_on_Z(const Cylinder3D &c){
  Vector3D main_dir = c.get_segment().get_direction();
  Vector3D vertical_dir = orthogonal_vector(main_dir);
  Transformation3D move2zero= Transformation3D(
                   get_identity_rotation_3d(),
                   -c.get_segment().get_middle_point());
  //transformation_from_reference_frame(a,b,c) , sets the Z-axis to
  //be prependicular to a and b. We want Z to be the main direction of
  //the cylinder
  Transformation3D rigid_trans=get_transformation_from_reference_frame(
                                 vertical_dir,
                                 vector_product(main_dir,vertical_dir),
                                 Vector3D(0.0,0.0,0.0));
  return rigid_trans.get_inverse();
}
}

Vector3Ds get_uniform_surface_cover(const Cylinder3D &cyl,
                                    int number_of_points) {
  Vector3Ds points;
  Vector3D starting_point,rotated_point;
  Vector3D z_direction(0.0,0.0,1.0);
  // move the cylinder to the base reference frame (center at (0,0,0)
  // and its main direction to be on the Z axis)
  Transformation3D cyl_rf_to_base_rf =
    get_transformation_to_place_direction_on_Z(cyl);
  Transformation3D move2zero =
    Transformation3D(get_identity_rotation_3d(),
                     -cyl.get_segment().get_middle_point());
  for(int i=0;i<number_of_points;i++) {
    ::boost::uniform_real<> rand(0,cyl.get_segment().get_length());
    starting_point = cyl.get_segment().get_point(0);
    starting_point = move2zero.get_transformed(starting_point);
    starting_point = cyl_rf_to_base_rf.get_transformed(starting_point);
    starting_point = starting_point +
               Vector3D(cyl.get_radius(),0.0,rand(random_number_generator));
   points.push_back(starting_point);
  }
  for(int i=0;i<number_of_points;i++) {
    ::boost::uniform_real<> rand(0,2*PI);
    //generate a random rotation around the cycle
    Rotation3D rot = get_rotation_in_radians_about_axis(z_direction,
                                rand(random_number_generator));
    rotated_point =  rot.get_rotated(points[i]);
    //back transformation of the rotated point back to the original cylinder
    rotated_point =  cyl_rf_to_base_rf.get_inverse()
      .get_transformed(rotated_point);
    rotated_point =  move2zero.get_inverse().get_transformed(rotated_point);
    points[i]=rotated_point;
  }
  return points;
}

Vector3Ds get_grid_surface_cover(const Cylinder3D &cyl,
                                 int number_of_cycles,
                                 int number_of_points_on_cycle){
  Vector3Ds points;
  // move the cylinder to the base reference frame (center at (0,0,0)
  // and its main direction to be on the Z axis)
  Transformation3D cyl_rf_to_base_rf =
    get_transformation_to_place_direction_on_Z(cyl);
  Vector3D z_direction(0.0,0.0,1.0);
  Float translation_step = cyl.get_segment().get_length()/number_of_cycles;
  Float rotation_step = 2*PI/number_of_points_on_cycle;
  std::vector<Rotation3D> rotations;
  for(int angle_ind = 0; angle_ind<number_of_points_on_cycle;angle_ind++) {
    rotations.push_back(
         get_rotation_in_radians_about_axis(z_direction,
                                            angle_ind*rotation_step));
  }
  Vector3D starting_point,rotated_point;
  Transformation3D move2zero =
    Transformation3D(get_identity_rotation_3d(),
                     -cyl.get_segment().get_middle_point());
  for(int cycle_ind = 0; cycle_ind<number_of_cycles;cycle_ind++) {
    starting_point = cyl.get_segment().get_point(0);
    starting_point = move2zero.get_transformed(starting_point);
    starting_point = cyl_rf_to_base_rf.get_transformed(starting_point);
    starting_point = starting_point +
              Vector3D(cyl.get_radius(),0.0,translation_step*cycle_ind);
    for(std::vector<Rotation3D>::iterator i_rot = rotations.begin();
        i_rot != rotations.end();i_rot++) {
      rotated_point =  i_rot->get_rotated(starting_point);
      //back transformation of the rotated point back to the original cylindar
      rotated_point =  cyl_rf_to_base_rf.get_inverse()
        .get_transformed(rotated_point);
      rotated_point =  move2zero.get_inverse().get_transformed(rotated_point);
      points.push_back(rotated_point);
    }
  }
  return points;
}


Vector3Ds get_uniform_surface_cover(const SpherePatch3D &sph,
                                    unsigned int number_of_points) {
  Vector3Ds points;
  while (points.size() < number_of_points) {
    Vector3D rp = get_random_vector_on(sph.get_sphere());
    double r2= (rp-sph.get_sphere().get_center()).get_squared_magnitude();
    IMP_INTERNAL_CHECK(std::abs(r2- square(sph.get_sphere().get_radius()))
                       < .05 *r2,
               "Bad point on sphere " << r2
               << " " << square(sph.get_sphere().get_radius()) << std::endl);
    // suppress warning
    if (0) {r2=r2+2;}
    if (sph.get_contains(rp)) {
      points.push_back(rp);
    }
  }
  return points ;
}

Vector3Ds get_uniform_surface_cover(const Cone3D &cone,
                        unsigned int number_of_points) {
 Vector3Ds points;
 Vector3D sph_p;
 Sphere3D sph(cone.get_tip(), std::sqrt(square(cone.get_radius())
                                        +square(cone.get_height())));
 while (points.size() < number_of_points) {
   sph_p=get_random_vector_in(sph);
   if (cone.get_contains(sph_p)) {
     points.push_back(sph_p);
   }
 }
 return points;
}



Vector3Ds get_random_chain(unsigned int n, double r,
                           const Vector3D &start,
                           const Sphere3Ds &obstacles) {
  IMP_USAGE_CHECK(r>.00001,
            "If r is too small, it won't succeed in placing the spheres");
  unsigned int max_failures=30;
  Vector3Ds ret;
  std::vector<unsigned int> failures;
  ret.push_back(start);
  failures.push_back(0);
  while (ret.size() != n) {
    if (ret.empty()) {
      IMP_FAILURE("Cannot place first random point");
    }
    if (failures.back() > max_failures) {
      IMP_LOG(VERBOSE, "Popping " << ret.back() << std::endl);
      ret.pop_back();
      failures.pop_back();
    }
    Vector3D v= get_random_vector_on(Sphere3D(ret.back(), 2*r));
    IMP_LOG(VERBOSE, "Trying " << v << " (" << ret.size() << ")"
            << std::endl);
    Sphere3D cb(v, r); // some slack
    bool bad=false;
    for (unsigned int i=0; i< ret.size()-1; ++i) {
      if (get_interiors_intersect(cb, Sphere3D(ret[i], r))) {
        bad=true;
        break;
      }
    }
    if (!bad) {
      for (unsigned int i=0; i< obstacles.size(); ++i) {
        if (get_interiors_intersect(obstacles[i], cb)) {
          bad=true;
          break;
        }
      }
    }
    if (bad) {
      ++failures.back();
    } else {
      ret.push_back(v);
      failures.push_back(0);
    }
  }
  return ret;
}


IMPALGEBRA_END_NAMESPACE
