/*
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/example/ExampleRestraint.h>

using namespace IMP;
int main() {
  set_log_level(VERBOSE);
  WeakPointer<Model> m(new Model());
  Particle* p0 = new Particle(m);
  core::XYZ d0 = core::XYZ::setup_particle(p0);
  Particle* p1 = new Particle(m);
  core::XYZ d1 = core::XYZ::setup_particle(p1);
  example::ExampleRestraint* r0 =
      new example::ExampleRestraint(m, p0->get_index(), 1);
  example::ExampleRestraint* r1 =
      new example::ExampleRestraint(m, p1->get_index(), 1);
  d0.set_coordinates(algebra::VectorD<3>(0, 0, 2));
  d1.set_coordinates(algebra::VectorD<3>(0, 0, 3));

  return 0;
}
