/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;


void test(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc, ());
  lpc->set_is_editing(true);
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  lpc->set_is_editing(false);
  IMP_NEW(DistancePairScore, dps, (new HarmonicLowerBound(0, 1)));
  IMP_NEW(PairsRestraint, pr, (dps, lpc));
  m->add_restraint(pr);
  double runtime=0, total=0;
  IMP_TIME(
             {
               total+=m->evaluate(true);
             }, runtime);
  std::ostringstream oss;
  oss << "evaluate " << n;
  IMP::benchmark::report(oss.str(), runtime, total);
}

void test_set(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc0, ());
  lpc0->set_is_editing(true);
  for (unsigned int i=0; i< ps.size()/2; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc0->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  lpc0->set_is_editing(false);

  IMP_NEW(ListPairContainer, lpc1, ());
  lpc1->set_is_editing(true);
  for (unsigned int i=ps.size()/2; i< ps.size(); ++i) {
    for (unsigned int j=ps.size()/2; j< i; ++j) {
      lpc1->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  lpc1->set_is_editing(false);
  IMP_NEW(PairContainerSet, pcs, ());
  pcs->add_pair_container(lpc0);
  pcs->add_pair_container(lpc1);

  IMP_NEW(DistancePairScore, dps, (new HarmonicLowerBound(0, 1)));
  IMP_NEW(PairsRestraint, pr, (dps, pcs));
  m->add_restraint(pr);
  double runtime=0, total=0;
  IMP_TIME(
             {
               total+=m->evaluate(true);
             }, runtime);
  std::ostringstream oss;
  oss << "evaluate set" << n;
  IMP::benchmark::report(oss.str(), runtime, total);
}



int main(int argc, char **argv) {
  test(100);
  test(1000);
  test_set(100);
  test_set(1000);
  return 0;
}
