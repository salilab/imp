/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/base/compiler_macros.h>
IMP_CLANG_PRAGMA(diagnostic ignored "-Wc++11-extensions")
#include <CGAL/Cartesian.h>
#include <CGAL/Min_sphere_of_spheres_d.h>
#include <cmath>
#include <IMP/cgal/internal/bounding_sphere.h>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE

algebra::Sphere3D
get_enclosing_sphere(const algebra::Sphere3Ds &ss) {
  IMP_USAGE_CHECK(!ss.empty(),
                  "Must pass some spheres to have a bounding sphere");
  typedef CGAL::Cartesian<double> K;
  typedef CGAL::Min_sphere_of_spheres_d_traits_3<K, K::RT> Traits;
  typedef CGAL::Min_sphere_of_spheres_d<Traits> Min_sphere;
  typedef K::Point_3                        Point;
  typedef Traits::Sphere                    Sphere;

  base::Vector<Sphere> spheres;
  for (unsigned int i=0; i< ss.size(); ++i) {
    // need cast to resolve ambiguity
    spheres.push_back(Sphere(Point(ss[i].get_center()[0],
                                   ss[i].get_center()[1],
                                   ss[i].get_center()[2]),
                             // this really is just radius
                             (ss[i].get_radius())));
  }
  Min_sphere ms(spheres.begin(), spheres.end());
  double x=*ms.center_cartesian_begin();
  double y=*(ms.center_cartesian_begin()+1);
  double z=*(ms.center_cartesian_begin()+2);
  double r=CGAL::to_double(ms.radius());
  algebra::Sphere3D s(algebra::Vector3D(x,y,z), r);
   /*IMP_IF_LOG(VERBOSE) {
     IMP_LOG_VERBOSE( "Enclosing sphere is " << s << " for ");
     for (unsigned int i=0; i< ss.size(); ++i) {
       IMP_LOG_VERBOSE( ss[i] << "| ");
     }
     IMP_LOG_VERBOSE( std::endl);
     }*/
   return s;

}
IMPCGAL_END_INTERNAL_NAMESPACE
