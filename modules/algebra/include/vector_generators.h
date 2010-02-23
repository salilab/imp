/**
 *  \file vector_generators.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_GENERATORS_H
#define IMPALGEBRA_VECTOR_GENERATORS_H

#include "VectorD.h"
#include "Cylinder3D.h"
#include "Cone3D.h"
#include "Sphere3D.h"
#include "SpherePatch3D.h"
#include "BoundingBoxD.h"

#include "internal/internal_vector_generators.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** @name Vector Generators

    These functions generate vector objects. Some
    of the methods, those with random in their name, generate
    a single vector chosen uniformly from the specified domain.
    Others, the cover methods, generate a set of points distributed
    (somewhat) evenly over the domain.
    @{
 */

//! Generate a random vector in a box with uniform density
/** \relatesalso BoundingBoxD
    \relatesalso VectorD
 */
template <unsigned int D>
VectorD<D>
get_random_vector_in(const BoundingBoxD<D> &bb) {
  VectorD<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    ::boost::uniform_real<> rand(bb.get_corner(0)[i],
                                 bb.get_corner(1)[i]);
    ret[i]=rand(random_number_generator);
  }
  return ret;
}

//! Generate a random vector on a box with uniform density
/** \relatesalso BoundingBoxD
    \relatesalso VectorD
 */
template <unsigned int D>
VectorD<D>
get_random_vector_on(const BoundingBoxD<D> &bb) {
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
  sv= get_random_vector_in(BoundingBoxD<D-1>(fmin, fmax));

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


//! Generate a random vector in a sphere with uniform density
/** \relatesalso VectorD
    \relatesalso SphereD
 */
template <unsigned int D>
VectorD<D>
get_random_vector_in(const SphereD<D> &s){
  BoundingBoxD<D> bb= get_bounding_box(s);
  double norm;
  VectorD<D> ret;
  double r2= square(s.get_radius());
  // \todo This algorithm could be more efficient.
  do {
    ret=get_random_vector_in(bb);
    norm= (s.get_center()- ret).get_squared_magnitude();
  } while (norm > r2);
  return ret;
}

//! Generate a random vector on a sphere with uniform density
/** \relatesalso VectorD
    \relatesalso SphereD
 */
template <unsigned int D>
VectorD<D>
get_random_vector_on(const SphereD<D> &s) {
  // could be made general
  BOOST_STATIC_ASSERT(D>0);
  double cur_radius2=square(s.get_radius());
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

  IMP_INTERNAL_CHECK(std::abs(up.get_magnitude() -s.get_radius()) < .1,
                     "Error generating vector on sphere: "
                     << up << " for " << s.get_radius());
  //IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);

  return s.get_center()+ up;
}


//! Generate a set of vectors which covers a sphere uniformly
/** The function is currently pretty slow, especially in non-optimized
    builds. Complain if this bugs you. We might be able to do better,
    at least in 3D.

    Creates at least the requested number of points.
    \cgalpredicate

    \relatesalso VectorD
    \relatesalso SphereD
    */
template <unsigned int D>
std::vector<VectorD<D> >
get_uniform_surface_cover(const SphereD<D> &s, unsigned int n) {
  return internal::uniform_cover_sphere(n, s.get_center(),
                                        s.get_radius(), true);
}


//! Generate a set of 3d points that uniformly cover a cylinder
/** \relatesalso VectorD
    \relatesalso Cylinder3D
*/
IMPALGEBRAEXPORT std::vector<VectorD<3> >
get_uniform_surface_cover(const Cylinder3D &cyl,
                          int number_of_points);

//! Generate a set of 3D points that uniformly cover a hemisphere
/** The points all lie on the upper hemisphere, eg, all their
    z coordinates are greater than those of the center of the sphere.
 */
template <unsigned int D>
std::vector<VectorD<D> >
get_uniform_upper_hemisphere_cover(const SphereD<D> &s, unsigned int n) {
  return internal::uniform_cover_sphere(n, s.get_center(),
                                        s.get_radius(), false);
}

//! Generate a grid of 3d points on a cylinder surface
/** \relatesalso Vector3D
    \relatesalso Cylinder3D
*/
IMPALGEBRAEXPORT  std::vector<VectorD<3> >
get_grid_surface_cover(const Cylinder3D &cyl,
                       int number_of_cycles,
                       int number_of_points_on_cycle);



//! Generate a set of 3d points that uniformly cover a patch of a sphere
/**
   \note the implementation can be improved
   \relatesalso SpherePatch3D
   \relatesalso VectorD
 */
IMPALGEBRAEXPORT  std::vector<VectorD<3> >
get_uniform_surface_cover(const SpherePatch3D &sph,
                          unsigned int number_of_points);

/** \relatesalso VectorD
    \relatesalso Cone3D
*/
IMPALGEBRAEXPORT  std::vector<VectorD<3> >
get_uniform_surface_cover(const Cone3D &cone,
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
IMPALGEBRAEXPORT  std::vector<VectorD<3> >
get_random_chain(unsigned int n, double r,
                 const VectorD<3> &start
                 = Vector3D(0,0,0),
                 const std::vector<SphereD<3> > &obstacles
                 =Sphere3Ds());

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_GENERATORS_H */
