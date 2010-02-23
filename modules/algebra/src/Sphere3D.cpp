/**
 *  \file  SphereD<3>.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP/base_types.h>
#include <IMP/algebra/Sphere3D.h>
#include <cmath>

#ifdef IMP_USE_CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Min_sphere_of_spheres_d.h>
#endif

IMPALGEBRA_BEGIN_NAMESPACE

SphereD<3> get_enclosing_sphere(const std::vector<SphereD<3> > &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
#ifdef IMP_USE_CGAL
  typedef CGAL::Exact_predicates_inexact_constructions_kernel             K;
  typedef CGAL::Min_sphere_of_spheres_d_traits_3<K, K::FT> Traits;
  typedef CGAL::Min_sphere_of_spheres_d<Traits> Min_sphere;
  typedef K::Point_3                        Point;
  typedef Traits::Sphere                    Sphere;

  std::vector<Sphere> spheres;
  for (unsigned int i=0; i< ss.size(); ++i) {
    // need cast to resolve ambiguity
    spheres.push_back(Sphere(Point(ss[i].get_center()[0],
                                   ss[i].get_center()[1],
                                   ss[i].get_center()[2]),
                             ss[i].get_radius()));
  }
  Min_sphere ms(spheres.begin(), spheres.end());
  SphereD<3> s(VectorD<3>(*ms.center_cartesian_begin(),
                      *(ms.center_cartesian_begin()+1),
                      *(ms.center_cartesian_begin()+2)),
              ms.radius());
   /*IMP_IF_LOG(VERBOSE) {
     IMP_LOG(VERBOSE, "Enclosing sphere is " << s << " for ");
     for (unsigned int i=0; i< ss.size(); ++i) {
       IMP_LOG(VERBOSE, ss[i] << "| ");
     }
     IMP_LOG(VERBOSE, std::endl);
     }*/
   return s;
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

IMPALGEBRA_END_NAMESPACE
