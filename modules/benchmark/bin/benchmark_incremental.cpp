/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;

double update_coordinates(Model *m, const Particles &ps) {
  double t=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZ xyz(ps[i]);
    xyz.set_coordinates(random_vector_in_box(Vector3D(0,0,0),
                                             Vector3D(10,10,10)));
    t+= xyz.get_coordinate(0)+xyz.get_coordinate(1)+xyz.get_coordinate(2);
  }
  t+=m->evaluate(false);
  return t;
}

void benchmark_baseline() {
  IMP_NEW(Model, m, ());
  unsigned int nump=100;
  Particles ps(nump);
  for (unsigned int i=0; i< nump; ++i) {
    IMP_NEW(Particle, p, (m));
    ps[i]=p;
    XYZ::setup_particle(p, Vector3D(i,2*i, 3*i));
    IMP_INTERNAL_CHECK(ps[i]==p, "not set");
  }
  {
    double nonincremental_time, incremental_time;
    double t=0, ti=0;
    IMP_TIME(t+=update_coordinates(m, ps), nonincremental_time);
    benchmark::report("changing nonincremental", nonincremental_time, t);
    m->set_is_incremental(true);
    IMP_TIME(ti+=update_coordinates(m, ps), incremental_time);
    benchmark::report("changing incremental", incremental_time, t);
  }
  ParticlePairs pps(ps.size()-1);
  for (unsigned int i=1; i< ps.size(); ++i) {
    pps[i-1]= ParticlePair(ps[i], ps[i-1]);
    IMP_INTERNAL_CHECK(pps[i-1][0]== ps[i], "Not set: "
                       << pps[i-1] << " " << ps[i]
                       << " " << pps[0]);
  }
  IMP_NEW(ListPairContainer, lpc, (pps));
  IMP_NEW(PairsRestraint, pr, (new DistancePairScore(new Harmonic(0,1)), lpc));
  m->add_restraint(pr);
  {
    double scored_nonincremental_time, scored_incremental_time;
    double scored_t=0, scored_ti=0;
    m->set_is_incremental(false);
    IMP_TIME(scored_t+=update_coordinates(m, ps), scored_nonincremental_time);
    benchmark::report("changing nonincremental score",
                      scored_nonincremental_time, scored_t);
    m->set_is_incremental(true);
    IMP_TIME(scored_ti+=update_coordinates(m, ps), scored_incremental_time);
    benchmark::report("changing incremental score",
                      scored_incremental_time, scored_ti);
  }
}

int main() {
  set_log_level(SILENT);
  benchmark_baseline();
  return 0;
}
