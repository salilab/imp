/**
 *  \file  SphereD<3>.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP/base_types.h>
#include <IMP/algebra/Sphere3D.h>
#include <cmath>
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/bounding_sphere.h>
#include <IMP/cgal/internal/union_of_balls.h>
#endif

IMPALGEBRA_BEGIN_NAMESPACE

SphereD<3> get_enclosing_sphere(const std::vector<SphereD<3> > &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  return cgal::internal::get_enclosing_sphere(ss);
#else
   BoundingBox3D bb= get_bounding_box(ss[0]);
   for (unsigned int i=1; i< ss.size(); ++i) {
     bb+= get_bounding_box(ss[i]);
   }
   VectorD<3> c= .5*(bb.get_corner(0)+ bb.get_corner(1));
   double r=0;
   for (unsigned int i=0; i< ss.size(); ++i) {
     double d= (c- ss[i].get_center()).get_magnitude();
     d+= ss[i].get_radius();
     r= std::max(r, d);
   }
   return SphereD<3>(c, r);
#endif
}

SphereD<3> get_enclosing_sphere(const std::vector<VectorD<3> > &vs) {
  std::vector<SphereD<3> > ss(vs.size());
  for (unsigned int i=0; i< vs.size(); ++i) {
    ss[i]= SphereD<3>(vs[i], 0);
  }
  return get_enclosing_sphere(ss);
}

#ifdef IMP_ALGEBRA_USE_IMP_CGAL
std::pair<double, double>
get_surface_area_and_volume(const std::vector<algebra::SphereD<3> > &ss) {
  return IMP::cgal::internal::get_surface_area_and_volume(ss);
}
#endif


IMPALGEBRA_END_NAMESPACE
