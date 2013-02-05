/**
 *  \file IMP/algebra/vector_generators.h
 *  \brief Functions to generate vectors.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include "utility.h"
#include "internal/grid_range_d.h"
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
template <int D>
inline VectorD<D>
get_random_vector_in(const BoundingBoxD<D> &bb) {
  return internal::get_random_vector_in(bb);
}

//! Generate a random vector on a box with uniform density
/** \relatesalso BoundingBoxD
    \relatesalso VectorD
 */
template <int D>
inline VectorD<D>
get_random_vector_on(const BoundingBoxD<D> &bb) {
  return internal::RandomVectorOnBB<D>::get(bb);
}


//! Generate a random vector in a sphere with uniform density
/** \relatesalso VectorD
    \relatesalso SphereD
 */
template <int D>
inline VectorD<D>
get_random_vector_in(const SphereD<D> &s){
  BoundingBoxD<D> bb= get_bounding_box(s);
  double norm;
  VectorD<D> ret;
  double r2= get_squared(s.get_radius());
  // \todo This algorithm could be more efficient.
  do {
    ret=get_random_vector_in(bb);
    norm= (s.get_center()- ret).get_squared_magnitude();
  } while (norm > r2);
  return ret;
}

/** Generates a random vector in a circle
    with uniform density with respect to the area of the circle

    @param s a 2D sphere (circle)

    \relatesalso VectorD
    \relatesalso SphereD
*/
IMPALGEBRAEXPORT
VectorD<2>
get_random_vector_in(const SphereD<2> &s);


//! Generate a random vector in a cylinder with uniform density
/** \relatesalso VectorD
    \relatesalso Cylinder3D
 */
IMPALGEBRAEXPORT
Vector3D
get_random_vector_in(const Cylinder3D &c);


//! Generate a random vector on a sphere with uniform density
/** \relatesalso VectorD
    \relatesalso SphereD
 */
template <int D>
inline VectorD<D>
get_random_vector_on(const SphereD<D> &s) {
  return internal::get_random_vector_on(s);
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
template <int D>
inline base::Vector<VectorD<D> >
get_uniform_surface_cover(const SphereD<D> &s, unsigned int n) {
  return internal::uniform_cover_sphere(n, s.get_center(),
                                        s.get_radius(), true);
}


//! Generate a set of 3d points that uniformly cover a cylinder
/** \relatesalso VectorD
    \relatesalso Cylinder3D
*/
IMPALGEBRAEXPORT Vector3Ds
get_uniform_surface_cover(const Cylinder3D &cyl,
                          int number_of_points);

//! Generate a set of 3D points that uniformly cover a hemisphere
/** The points all lie on the upper hemisphere, eg, all their
    z coordinates are greater than those of the center of the sphere.
 */
template <int D>
inline base::Vector<VectorD<D> >
get_uniform_upper_hemisphere_cover(const SphereD<D> &s, unsigned int n) {
  return internal::uniform_cover_sphere(n, s.get_center(),
                                        s.get_radius(), false);
}

//! Generate a grid of 3d points on a cylinder surface
/** \relatesalso Vector3D
    \relatesalso Cylinder3D
*/
IMPALGEBRAEXPORT  Vector3Ds
get_grid_surface_cover(const Cylinder3D &cyl,
                       int number_of_cycles,
                       int number_of_points_on_cycle);



//! Generate a set of 3d points that uniformly cover a patch of a sphere
/**
   \note the implementation can be improved
   \relatesalso SpherePatch3D
   \relatesalso VectorD
 */
IMPALGEBRAEXPORT  Vector3Ds
get_uniform_surface_cover(const SpherePatch3D &sph,
                          unsigned int number_of_points);

/** \relatesalso VectorD
    \relatesalso Cone3D
*/
IMPALGEBRAEXPORT  Vector3Ds
get_uniform_surface_cover(const Cone3D &cone,
                          unsigned int number_of_points);

/** Cover the interior of the bounding box by equal sized
    parallelograms of approximately full-width s, returning the
    list of centers of the cubes.
 */
template <int D>
base::Vector<VectorD<D> >
get_grid_interior_cover_by_spacing(const BoundingBoxD<D> &bb, double s) {
  const unsigned int dim= bb.get_dimension();
  Ints ns(dim);
  algebra::VectorD<D> start(bb.get_corner(0));
  algebra::VectorD<D> spacing(bb.get_corner(0));
  for (unsigned int i=0; i< dim; ++i) {
    double w= bb.get_corner(1)[i]- bb.get_corner(0)[i];
    if (w < s) {
      start[i]= bb.get_corner(0)[i]+w*.5;
      spacing[i]=1;
      ns[i]=1;
    } else {
      ns[i]= static_cast<int>(std::floor(w/s));
      spacing[i]= w/ns[i];
      start[i]=bb.get_corner(0)[i]+spacing[i]*.5;
    }
  }
  Ints cur(D,0);
  base::Vector<VectorD<D> > ret;
  do {
    ret.push_back(start+get_elementwise_product(cur, spacing));
    unsigned int i;
    for (i=0; i< dim; ++i) {
      ++cur[i];
      if (cur[i]==ns[i]) {
        cur[i]=0;
      } else {
        break;
      }
    }
    if (i==dim) break;
  } while(true);
  return ret;
}


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
IMPALGEBRAEXPORT  Vector3Ds
get_random_chain(unsigned int n, double r,
                 const Vector3D &start
                 = Vector3D(0,0,0),
                 const Sphere3Ds &obstacles
                 =Sphere3Ds());

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_GENERATORS_H */
