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
#include "BoundingBoxD.h"

#include "internal/vector_generators.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** @name Vector Generators

    These functions generate VectorD or Vector3D objects using
    a variety of distributions.
    @{
 */

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
random_vector_in_box(const BoundingBoxD<D> &bb) {
  VectorD<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    ::boost::uniform_real<> rand(bb.get_corner(0)[i],
                                 bb.get_corner(1)[i]);
    ret[i]=rand(random_number_generator);
  }
  return ret;
}

//! Generate a random vector on a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_box(const BoundingBoxD<D> &bb) {
  double areas[D*2];
  VectorD<D> lb= bb.get_corner(0);
  VectorD<D> ub= bb.get_corner(1);
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
  sv= random_vector_in_box(BoundingBoxD<D-1>(fmin, fmax));

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


/** Generate a random vector in a box with uniform density.

    The box defined by the corners min, max must be properly
    oriented and non-empty.
 */
template <unsigned int D>
VectorD<D>
random_vector_in_box(const VectorD<D>&min,
                     const VectorD<D>&max) {
  return random_vector_in_box(BoundingBoxD<D>(min,max));
}

/** Generate a random vector in a box with uniform density.

    The box defined by the corners min, max must be properly
    oriented and non-empty.
 */
template <unsigned int D>
VectorD<D>
random_vector_on_box(const VectorD<D>&min,
                     const VectorD<D>&max) {
  return random_vector_on_box(BoundingBoxD<D>(min,max));
}


//! Generate a random vector in a box with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_box() {
  return random_vector_in_box(BoundingBoxD<D>(VectorD<D>(0,0,0),
                                              VectorD<D>(1,1,1)));
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
    ret=random_vector_in_box(BoundingBoxD<D>(min, max));
    norm= (center- ret).get_squared_magnitude();
  } while (norm > r2);
  return ret;
}

//! Generate a random vector in a unit sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_in_unit_sphere(){
  return random_vector_in_sphere(zeros<D>(), 1);
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
  //IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);

  return center+ up;
}



//! Generate a random vector on a sphere with uniform density
template <unsigned int D>
VectorD<D>
random_vector_on_unit_sphere() {
  return random_vector_on_sphere(zeros<D>(), 1);
}


//! Generate a set of vectors which covers a sphere uniformly
/** The function is currently pretty slow, especially in non-optimized
    builds. Complain if this bugs you. We might be able to do better,
    at least in 3D.

    Creates at least the requested number of points.
    \cgalpredicate
    */
template <unsigned int D>
std::vector<VectorD<D> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<D> &center,
                     double radius) {
  return internal::uniform_cover_sphere<D, false>(n, center, radius);
}


//! Generate a set of 3d points that uniformly cover a cylinder
IMPALGEBRAEXPORT Vector3Ds uniform_cover(const Cylinder3D &cyl,
                        int number_of_points);

//! Generate a grid of 3d points on a cylinder surface
IMPALGEBRAEXPORT Vector3Ds grid_cover(const Cylinder3D &cyl,
                                      int number_of_cycles,
                                      int number_of_points_on_cycle);

//! Generate a set of 3d points that uniformly cover a sphere
/** Creates at least number_of_points points. */
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


//! Generate a random chain with no collisions
/** This function generates a random chain, starting at (0,0,0)
    with n particles each with radius r. Consecutive particles are
    approximately distance 2r apart and no pair of particles is closer
    than 2r.

    If an obstacles parameter is provided then chain spheres also don't
    intersect the obstacle spheres.

    \note The current implementation is not very clever and can be made
    more clever if needed.
 */
IMPALGEBRAEXPORT Vector3Ds random_chain(unsigned int n, double r,
                                        const Vector3D &start
                                        = Vector3D(0,0,0),
                                        const Sphere3Ds &obstacles
                                        =Sphere3Ds());

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_GENERATORS_H */
