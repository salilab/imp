/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;

void time_both(PairContainer *pc, PairScore *ps, std::string name) {
  {
    double runtime=0, total=0;
    IMP_TIME(
             {
               total+=pc->evaluate(ps, NULL);
             }, runtime);
    std::ostringstream oss;
    oss << "container " << name << " in "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, total);
  }
  {
   double runtime=0, total=0;
    IMP_TIME(
             {
               for (unsigned int i=0; i< pc->get_number_of_particle_pairs();
                    ++i) {
                 total+= ps->evaluate(pc->get_particle_pair(i), NULL);
               }
             }, runtime);
    std::ostringstream oss;
    oss << "container " << name << " out "
        << pc->get_number_of_particle_pairs();
    IMP::benchmark::report(oss.str(), runtime, total);
  }
}

void test(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc, ());
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  IMP_NEW(DistancePairScore, dps, (new HarmonicLowerBound(0, 1)));
  time_both(lpc, dps, "list");
}

void test_set(int n) {
  set_log_level(SILENT);
  IMP_NEW(Model, m, ());
  Particles ps= create_xyzr_particles(m, n, .1);
  IMP_NEW(ListPairContainer, lpc0, ());
  for (unsigned int i=0; i< ps.size()/2; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      lpc0->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }

  IMP_NEW(ListPairContainer, lpc1, ());
  for (unsigned int i=ps.size()/2; i< ps.size(); ++i) {
    for (unsigned int j=ps.size()/2; j< i; ++j) {
      lpc1->add_particle_pair(ParticlePair(ps[i], ps[j]));
    }
  }
  IMP_NEW(PairContainerSet, pcs, ());
  pcs->add_pair_container(lpc0);
  pcs->add_pair_container(lpc1);

  IMP_NEW(DistancePairScore, dps, (new HarmonicLowerBound(0, 1)));
  time_both(pcs, dps, "set");
}



int main(int argc, char **argv) {
  test(100);
  test(1000);
  test_set(100);
  test_set(1000);
  return 0;
}
