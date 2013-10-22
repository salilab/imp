/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Sphere3D.h"
#include <IMP/base/exception.h>
#include <utility>
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/bounding_sphere.h>
#include <IMP/cgal/internal/union_of_balls.h>
#endif

IMPALGEBRA_BEGIN_NAMESPACE

Sphere3D get_enclosing_sphere(const Sphere3Ds &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  return cgal::internal::get_enclosing_sphere(ss);
#else
  BoundingBox3D bb = get_bounding_box(ss[0]);
  for (unsigned int i = 1; i < ss.size(); ++i) {
    bb += get_bounding_box(ss[i]);
  }
  Vector3D c = .5 * (bb.get_corner(0) + bb.get_corner(1));
  double r = 0;
  for (unsigned int i = 0; i < ss.size(); ++i) {
    double d = (c - ss[i].get_center()).get_magnitude();
    d += ss[i].get_radius();
    r = std::max(r, d);
  }
  return Sphere3D(c, r);
#endif
}

Sphere3D get_enclosing_sphere(const Vector3Ds &vs) {
  Sphere3Ds ss(vs.size());
  for (unsigned int i = 0; i < vs.size(); ++i) {
    ss[i] = Sphere3D(vs[i], 0);
  }
  return get_enclosing_sphere(ss);
}

#ifdef IMP_ALGEBRA_USE_IMP_CGAL
std::pair<double, double> get_surface_area_and_volume(
    const algebra::Sphere3Ds &ss) {
  return IMP::cgal::internal::get_surface_area_and_volume(ss);
}
#endif

IMPALGEBRA_END_NAMESPACE
