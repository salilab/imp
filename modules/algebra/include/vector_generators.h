/**
 *  \file vector_generators.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_GENERATORS_H
#define IMPALGEBRA_VECTOR_GENERATORS_H

#include "VectorD.h"
#include "Cylinder3D.h"
#include "Cone3D.h"
#include "Sphere3D.h"
#include "Sphere3DPatch.h"
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
  double r2= square(radius);
  // \todo This algorithm could be more efficient.
  do {
    ret=random_vector_in_box(min, max);
    norm= (center- ret).get_squared_magnitude();
  } while (norm > r2);
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
  double cur_radius2=square(radius);
  VectorD<D> up;
  for (unsigned int i=D-1; i>0; --i) {
    double r= std::sqrt(cur_radius2);
    ::boost::uniform_real<> rand(-r, r);
    up[i]= rand(random_number_generator);
    // radius of circle
    cur_radius2= cur_radius2-square(up[i]);
  }
  ::boost::uniform_int<> rand(0, 1);
  double x= std::sqrt(cur_radius2);
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
IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Cylinder3D &cyl,
                        int number_of_points);

//! Generate a grid of 3d points on a cylinder surface
IMPALGEBRAEXPORT Vector3Ds grid_cover(const Cylinder3D &cyl,
                                      int number_of_cycles,
                                      int number_of_points_on_cycle);

//! Generate a set of 3d points that uniformly cover a sphere
IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Sphere3D &sph,
                                         int number_of_points) ;
//! Generate a set of 3d points that uniformly cover a patch of a sphere
/**
/note the implementation can be improved
 */
IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Sphere3DPatch &sph,
                                         unsigned int number_of_points);

IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Cone3D &cone,
                                         unsigned int number_of_points);

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_GENERATORS_H */
