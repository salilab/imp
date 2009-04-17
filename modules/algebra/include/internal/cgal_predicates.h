/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_CGAL_PREDICATES_H
#define IMPALGEBRA_INTERNAL_CGAL_PREDICATES_H

#include "../config.h"
#include <IMP/base_types.h>
#ifdef IMP_USE_CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

typedef CGAL::Exact_predicates_inexact_constructions_kernel CGALKernel;

template <class Plane, class Vector>
inline int cgal_plane_compare_above(const Plane &p, const Vector&v) {
  typename CGALKernel::Point_3 pt(p.get_point_on_plane()[0],
                                  p.get_point_on_plane()[1],
                                  p.get_point_on_plane()[2]);
  typename CGALKernel::Vector_3 n(p.get_normal()[0],
                                  p.get_normal()[1],
                                  p.get_normal()[2]);
  typename CGALKernel::Plane_3 plane(pt, n);
  typename CGALKernel::Point_3 qpt(v[0], v[1], v[2]);
  return plane.oriented_side(qpt);
}

template <class Sphere, class Vector>
inline int cgal_sphere_compare_inside(const Sphere &p, const Vector&v) {
  typename CGALKernel::Point_3 pt(p.get_center()[0],
                                  p.get_center()[1],
                                  p.get_center()[2]);
  typename CGALKernel::Sphere_3 s(pt, square(p.get_radius()));
  typename CGALKernel::Point_3 qpt(v[0], v[1], v[2]);
  return p.bounded_side(qpt);
}


IMPALGEBRA_END_INTERNAL_NAMESPACE\

#endif

#endif  /* IMPALGEBRA_INTERNAL_CGAL_PREDICATES_H */
