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


int main(int argc, char **argv) {
  test(100);
  test(1000);
  return 0;
}
