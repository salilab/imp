/**
 *  \file XYZR.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/XYZR.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>
#include "IMP/core/Hierarchy.h"

IMPCORE_BEGIN_NAMESPACE

void XYZR::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
      << get_y() << ", " << get_z() << ": " << get_radius() << ")";

}


Float distance(XYZR a, XYZR b)
{
  return distance(a.get_sphere(), b.get_sphere());
}


void set_enclosing_sphere(XYZR out,
                          const Particles &v)
{
  algebra::Sphere3Ds ss(v.size());
  for (unsigned int i=0; i< v.size(); ++i) {
    XYZ d(v[i]);
    Float r=0;
    if (v[i]->has_attribute(out.get_radius_key())) {
      r= v[i]->get_value(out.get_radius_key());
    }
    ss[i]= algebra::Sphere3D(d.get_coordinates(), r);
  }
  algebra::Sphere3D s= algebra::enclosing_sphere(ss);
  out.set_sphere(s);
}


void set_enclosing_radius(XYZR out,
                          const Particles &v)
{
  double r=0;
  for (unsigned int i=0; i< v.size(); ++i) {
    if (XYZR::is_instance_of(v[i], out.get_radius_key())) {
      XYZR d(v[i]);
      double dist= distance(static_cast<XYZ>(out), static_cast<XYZ>(d));
      dist+= d.get_radius();
      r= std::max(dist, r);
    } else {
      XYZ d(v[i]);
      double dist= distance(static_cast<XYZ>(out), static_cast<XYZ>(d));
      r= std::max(dist, r);
    }
  }
  out.set_radius(r);
}


Particles create_xyzr_particles(Model *m,
                                unsigned int num,
                                Float radius,
                                Float box_side) {
  Particles ret;
  for (unsigned int i=0; i< num; ++i) {
    Particle *p= new Particle(m);
    XYZR d= XYZR::create(p);
    d.set_coordinates(algebra::random_vector_in_box(
                            algebra::Vector3D(-box_side, -box_side, -box_side),
                            algebra::Vector3D(box_side, box_side, box_side)));
    d.set_radius(radius);
    d.set_coordinates_are_optimized(true);
    ret.push_back(p);
  }
  return ret;
}



Hierarchy
create_sphere_hierarchy(const Particles &ps,
                        const HierarchyTraits& traits) {
  IMP_failure("Not Implemented", InvalidStateException);
  /* - if ps is empty, return
     - create a new XYZR and Hierarchy particle
     - set its enclosing sphere to the enclosing sphere of ps
     - find the largest extent of x,y,z
     - divide the points at the midpoint (of one or more directions)
     - recurse and add the two return values as children to the new particle
   */
  return Hierarchy();
}




IMPCORE_END_NAMESPACE
