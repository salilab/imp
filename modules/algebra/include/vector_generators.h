/**
 *  \file vector_generators.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_GENERATORS_H
#define IMPALGEBRA_VECTOR_GENERATORS_H

#include "VectorD.h"
#include "Cylinder3D.h"
#include "Sphere3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! create a constant vector
/** This is not the right name.
 */
template <unsigned int D>
VectorD<D> constant_vector(double s) {
  VectorD<D> ret;
  for (unsigned int i= 0; i < D; ++i) {
    ret[i]=s;
  }
  return ret;
}


//! Generate a random vector in a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_box(const VectorD<D> &lb,
                     const VectorD<D> &ub) {
  VectorD<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    IMP_check(lb[i] < ub[i], "Box for randomize must be non-empty",
              ValueException);
    ::boost::uniform_real<> rand(lb[i], ub[i]);
    ret[i]=rand(random_number_generator);
  }
  return ret;
}

//! Generate a random vector on a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_box(const VectorD<D> &lb,
                     const VectorD<D> &ub) {
  double areas[D*2];
  for (unsigned int i=0; i< D; ++i) {
    areas[i]=1;
    for (unsigned int j=1; j< D; ++j) {
      areas[i] *= ub[(i+j)%D]-lb[(i+j)%D];
    }
    if (i!= 0) {
      areas[i]+= areas[i-1];
    }
  }
  for (unsigned int i=0; i< D; ++i) {
    areas[D+i]= areas[D-1]+areas[i];
  }
  /*for (unsigned int i=0; i< D*2; ++i) {
    std::cout << areas[i] << " ";
    }*/
  ::boost::uniform_real<> rand(0, areas[2*D-1]);
  double a= rand(random_number_generator);
  //std::cout << ": " << a << std::endl;
  unsigned int side;
  for (side=0; side< 2*D; ++side) {
    if (areas[side] > a) break;
  }
  unsigned int coord= (side>=D? side-D: side);
  VectorD<D-1> fmin, fmax, sv;
  for (unsigned int i=1; i< D; ++i) {
    fmin[i-1]= 0;
    fmax[i-1]= ub[(coord+i)%D]- lb[(coord+i)%D];
  }
  sv= random_vector_in_box(fmin, fmax);

  VectorD<D> ret;
  //std::cout << "Side is " << side << std::endl;
  if (side >=D) {
    ret=ub;
    for (unsigned int i=1; i< D; ++i) {
      ret[(coord+i)%D]-= sv[i-1];
    }
  } else {
    ret=lb;
    for (unsigned int i=1; i< D; ++i) {
      ret[(coord+i)%D]+= sv[i-1];
    }
  }

  return ret;
}

//! Generate a random vector in a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_box() {
  return random_vector_in_box(VectorD<D>(0,0,0),
                              VectorD<D>(1,1,1));
}

//! Generate a random vector in a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_sphere(const VectorD<D> &center,
                        double radius){
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  VectorD<D> rad= constant_vector<D>(radius);
  VectorD<D> min= center - rad;
  VectorD<D> max= center + rad;
  double norm;
  VectorD<D> ret;
  // \todo This algorithm could be more efficient.
  do {
    ret=random_vector_in_box(min, max);
    norm= (center- ret).get_magnitude();
  } while (norm > radius);
  return ret;
}

//! Generate a random vector in a unit sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_sphere(){
  return random_vector_in_sphere(VectorD<D>(0,0,0), 1);
}

//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_sphere(const VectorD<D> &center,
                        double radius) {
  // could be made general
  BOOST_STATIC_ASSERT(D>0);
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  double cur_radius=radius;
  VectorD<D> up;
  for (unsigned int i=D-1; i>0; --i) {
    ::boost::uniform_real<> rand(-cur_radius,cur_radius);
    up[i]= rand(random_number_generator);
    // radius of circle
    cur_radius= std::sqrt(square(cur_radius)-square(up[i]));
  }
  ::boost::uniform_int<> rand(0, 1);
  double x= cur_radius;
  if (rand(random_number_generator)) {
    x=-x;
  }
  up[0]=x;

  IMP_assert(std::abs(up.get_magnitude() -radius) < .1,
             "Error generating vector on sphere: "
             << up << " for " << radius);
  IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);

  return center+ up;
}


//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_unit_sphere() {
  VectorD<D> v;
  for (unsigned int i=0; i < D; ++i) {
    v[i]=0;
  }
  return random_vector_on_sphere(v, 1);
}

//! Generate a set of 3d points that uniformly cover a cylinder
inline Vector3Ds uniform_cover(const Cylinder3D &cyl,
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
    ::boost::uniform_real<> rand(0,cyl.get_height());
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
//! Generate a grid of 3d points on a cylinder surface
inline Vector3Ds grid_cover(const Cylinder3D &cyl,
                     int number_of_cycles, int number_of_points_on_cycle){
  Vector3Ds points;
  // move the cylinder to the base reference frame (center at (0,0,0)
  // and its main direction to be on the Z axis)
  Transformation3D cyl_rf_to_base_rf =
    cyl.get_transformation_to_place_direction_on_Z();
  Vector3D z_direction(0.0,0.0,1.0);
  Float translation_step = cyl.get_height()/number_of_cycles;
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

//! Generate a set of 3d points that uniformly cover a cylinder
inline Vector3Ds uniform_cover(const Sphere3D &sph,int number_of_points) {
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


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_GENERATORS_H */
