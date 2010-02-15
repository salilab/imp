/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;


void test(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListSingletonContainer, lsc, (ps));
  for (unsigned int i=0; i< ps.size(); ++i) {
    Diffusion::setup_particle(ps[i], 1e-6);
  }
  IMP_NEW(ClosePairContainer, cpss, (lsc, 0.0, .1));

  IMP_NEW(DistancePairScore, dps, (new HarmonicLowerBound(0, 1)));
  IMP_NEW(PairsRestraint, pr, (dps, cpss));
  m->add_restraint(pr);
  SimulationParameters sp
    = SimulationParameters::setup_particle(new Particle(m));
  sp.set_maximum_time_step_in_femtoseconds(1e3);
  IMP_NEW(BrownianDynamics, bd, (sp, lsc));
  bd->set_model(m);
  double runtime=0, total=0;
  IMP_TIME(
             {
               total+=bd->simulate(1e6);
             }, runtime);
  std::ostringstream oss;
  oss << "bd " << n;
  IMP::benchmark::report(oss.str(), runtime, total);
}


int main(int argc, char **argv) {
  test(100);
  test(1000);
  return 0;
}
