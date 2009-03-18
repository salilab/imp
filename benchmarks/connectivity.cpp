/**
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <boost/timer.hpp>

using namespace IMP;
using namespace IMP::core;

int main() {
  unsigned int npart=1000;
  unsigned int reps=10;
  Model *m= new Model();
  UnaryFunction *ub = new HarmonicUpperBound(1.0, 0.1);
  PairScore *ss= new DistancePairScore(ub);
  Particles ps = create_xyzr_particles(m, npart, .1);
  ConnectivityRestraint* r= new ConnectivityRestraint(ss);
  r->set_particles(ps);
  m->add_restraint(r);
  set_check_level(EXPENSIVE);
  set_log_level(VERBOSE);
  m->evaluate(NULL);

  set_check_level(NONE);
  set_log_level(SILENT);
  boost::timer t;
  for (unsigned int i=0; i< 10; ++i) {
    m->evaluate(NULL);
  }
  std::cout << "Connectivity restraint on " << npart
            << " took " << t.elapsed()/reps
            << std::endl;
  return 0;
}
