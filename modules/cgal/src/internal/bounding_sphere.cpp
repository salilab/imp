/**
 *  \file  SphereD<3>.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/base_types.h>
#include <IMP/cgal/internal/bounding_sphere.h>
#include <cmath>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Min_sphere_of_spheres_d.h>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE

algebra::SphereD<3>
get_enclosing_sphere(const std::vector<algebra::SphereD<3> > &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
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
  algebra::SphereD<3> s(algebra::VectorD<3>(*ms.center_cartesian_begin(),
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

}


IMPCGAL_END_INTERNAL_NAMESPACE
