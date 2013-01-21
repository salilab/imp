/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/MSConnectivityRestraint.h>
#include <IMP/container/ConnectingPairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/container/generic.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>

using namespace IMP;
using namespace IMP::base;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::container;

namespace {
void benchmark_it(std::string name, std::string algorithm,
                  ListSingletonContainer *lsc, Model *m) {
  double runtime;
  double value=0;
  m->evaluate(false);
  IMP_TIME({
      IMP_CONTAINER_FOREACH(SingletonContainer, lsc,
                        {
                          XYZ(lsc->get_model(), _1)
          .set_coordinates(get_random_vector_in(BoundingBox3D(Vector3D(0,0,0),
                                                      Vector3D(10,10,10))));
                        });
      value+= m->evaluate(false);
    }, runtime);
  IMP::benchmark::report(name, algorithm, runtime, value);
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark connectivity");
  unsigned int npart=1000;
  set_log_level(SILENT);
  //set_check_level(NONE);
  IMP_NEW(Model, m, ());
  IMP_NEW(HarmonicUpperBoundSphereDistancePairScore, ss,(0, 1));
  ParticlesTemp ps = create_xyzr_particles(m, npart, .1);
  IMP_NEW(ListSingletonContainer, lsc, (ps));
  {
    ConnectivityRestraint* r= new ConnectivityRestraint(ss, lsc);
    m->add_restraint(r);
    benchmark_it("connectivity", "slow", lsc, m);
    m->remove_restraint(r);
  }
 {
    MSConnectivityRestraint* r= new MSConnectivityRestraint(ss);
    Ints composite;
    for (unsigned int i=0; i< ps.size(); ++i) {
      composite.push_back(r->add_type(ParticlesTemp(1, ps[i])));
    }
    r->add_composite(composite);
    m->add_restraint(r);
    benchmark_it("ms_connectivity", "slow", lsc, m);
    m->remove_restraint(r);
  }
  {
    IMP_NEW(ConnectingPairContainer, cpc,(lsc, .1));
    Pointer<Restraint> pr(container::create_restraint(ss.get(), cpc.get()));
    m->add_restraint(pr);
    benchmark_it("connectivity", "fast", lsc, m);
    m->remove_restraint(pr);
  }
  return IMP::benchmark::get_return_value();
}
