/**
 *  \file XYZRDecorator.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/XYZRDecorator.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>

#ifdef IMP_USE_CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Min_sphere_of_spheres_d.h>
#endif

IMPCORE_BEGIN_NAMESPACE

void XYZRDecorator::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
      << get_y() << ", " << get_z() << ": " << get_radius() << ")";

}


Float distance(XYZRDecorator a, XYZRDecorator b)
{
  return distance(a.get_sphere(), b.get_sphere());
}


void set_enclosing_sphere(const Particles &v,
                          XYZRDecorator out)
{
  IMP_check(!v.empty(), "Must pass some particles to have a bounding sphere",
            ValueException);
  for (unsigned int i=0; i< v.size(); ++i) {
    XYZDecorator::cast(v[i]);
  }
  FloatKey rk= out.get_radius_key();


#ifdef IMP_USE_CGAL
  typedef CGAL::Exact_predicates_inexact_constructions_kernel             K;
  typedef CGAL::Min_sphere_of_spheres_d_traits_3<K, K::FT> Traits;
  typedef CGAL::Min_sphere_of_spheres_d<Traits> Min_sphere;
  typedef K::Point_3                        Point;
  typedef Traits::Sphere                    Sphere;

  std::vector<Sphere> spheres;
  for (unsigned int i=0; i< v.size(); ++i) {
    // need cast to resolve ambiguity
    XYZDecorator d(v[i]);
    float r=0;
    if (v[i]->has_attribute(rk)) r= v[i]->get_value(rk);

    spheres.push_back(Sphere(Point(d.get_x(),
                                   d.get_y(),
                                   d.get_z()),
                             square(r)));
  }
  Min_sphere ms(spheres.begin(), spheres.end());

  out.set_radius(ms.radius());
  out.set_x(*ms.center_cartesian_begin());
  out.set_y(*(ms.center_cartesian_begin()+1));
  out.set_z(*(ms.center_cartesian_begin()+2));
#else
  algebra::Vector3D c(0,0,0);
  for (unsigned int i=0; i< v.size(); ++i) {
    XYZDecorator d(v[i]);
    c+= d.get_coordinates();
  }
  c/=v.size();
  out.set_coordinates(c);
  float r=0;
  for (unsigned int i=0; i< v.size(); ++i) {
    float d= distance(XYZDecorator(v[i]), out);
    if (v[i]->has_attribute(rk)) d+= v[i]->get_value(rk);
    r= std::max(r, d);
  }
  out.set_radius(r);
#endif
}


Particles create_xyzr_particles(Model *m,
                                unsigned int num,
                                Float radius,
                                Float box_side) {
  Particles ret;
  for (unsigned int i=0; i< num; ++i) {
    Particle *p= new Particle();
    m->add_particle(p);
    XYZRDecorator d= XYZRDecorator::create(p);
    d.set_coordinates(algebra::random_vector_in_box(
                            algebra::Vector3D(-box_side, -box_side, -box_side),
                            algebra::Vector3D(box_side, box_side, box_side)));
    d.set_radius(radius);
    d.set_coordinates_are_optimized(true);
    ret.push_back(p);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
