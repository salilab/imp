/**
 *  \file XYZRDecorator.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/XYZRDecorator.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>

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
  algebra::Sphere3Ds ss(v.size());
  for (unsigned int i=0; i< v.size(); ++i) {
    XYZDecorator d(v[i]);
    Float r=0;
    if (v[i]->has_attribute(out.get_radius_key())) {
      r= v[i]->get_value(out.get_radius_key());
    }
    ss[i]= algebra::Sphere3D(d.get_coordinates(), r);
  }
  algebra::Sphere3D s= algebra::enclosing_sphere(ss);
  out.set_sphere(s);
}


Particles create_xyzr_particles(Model *m,
                                unsigned int num,
                                Float radius,
                                Float box_side) {
  Particles ret;
  for (unsigned int i=0; i< num; ++i) {
    Particle *p= new Particle(m);
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
