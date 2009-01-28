/*
 * Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/examples/ExampleRestraint.h>
#include <IMP/core/ListPairContainer.h>

using namespace IMP;
int main(){
  set_log_level(VERBOSE);
  Pointer<Model> m(new Model());
  Particle* p0= new Particle(m);
  core::XYZDecorator d0= core::XYZDecorator::create(p0);
  Particle* p1= new Particle(m);
  core::XYZDecorator d1= core::XYZDecorator::create(p1);
  core::DistancePairScore *dps
    = new core::DistancePairScore(new core::Linear(0,1));
  core::ListPairContainer *pc= new core::ListPairContainer();
  pc->add_particle_pair(ParticlePair(p0, p1));
  examples::ExampleRestraint *r= new examples::ExampleRestraint(dps, pc);
  d0.set_coordinates(algebra::Vector3D(0,0,0));
  d1.set_coordinates(algebra::Vector3D(0,0,1));

  m->add_restraint(r);
  IMP_assert(std::abs(m->evaluate(false) -1) < .01, "Out of range");
  return 0;
}
