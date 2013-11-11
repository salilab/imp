/*
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/base/flags.h>
#include <IMP/example/ExampleRestraint.h>
#include <IMP/container/ListPairContainer.h>

using namespace IMP;
int main(int argc, char **argv) {
  base::setup_from_argv(argc, argv, "An example program");
  base::Pointer<kernel::Model> m(new kernel::Model());
  kernel::ParticleIndex p0 = m->add_particle("P0");
  core::XYZ d0 = core::XYZ::setup_particle(m, p0);
  kernel::ParticleIndex p1 = m->add_particle("P1");
  core::XYZ d1 = core::XYZ::setup_particle(m, p1);
  base::Pointer<example::ExampleRestraint> r0 =
      new example::ExampleRestraint(m, p0, 1);
  base::Pointer<example::ExampleRestraint> r1 =
      new example::ExampleRestraint(m, p1, 1);
  kernel::RestraintsTemp rs;
  rs.push_back(r0);
  rs.push_back(r1);
  d0.set_coordinates(algebra::Vector3D(0, 0, 2));
  d1.set_coordinates(algebra::Vector3D(0, 0, 3));

  base::Pointer<core::RestraintsScoringFunction> scoring_function =
      new core::RestraintsScoringFunction(rs);
  return 0;
}
