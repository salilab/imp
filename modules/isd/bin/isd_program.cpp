/*
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/isd/ExampleRestraint.h>
#include <IMP/container/ListPairContainer.h>

using namespace IMP;
int main(){
  set_log_level(VERBOSE);
  WeakPointer<Model> m(new Model());
  Particle* p0= new Particle(m);
  core::XYZ d0= core::XYZ::setup_particle(p0);
  Particle* p1= new Particle(m);
  core::XYZ d1= core::XYZ::setup_particle(p1);
  isd::ExampleRestraint *r0= new isd::ExampleRestraint(p0, 1);
  isd::ExampleRestraint *r1= new isd::ExampleRestraint(p1, 1);
  d0.set_coordinates(algebra::VectorD<3>(0,0,2));
  d1.set_coordinates(algebra::VectorD<3>(0,0,3));

  m->add_restraint(r0);
  m->add_restraint(r1);
  return 0;
}
