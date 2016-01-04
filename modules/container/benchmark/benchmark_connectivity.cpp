/**
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
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
#include <IMP/flags.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::container;

namespace {
void benchmark_it(std::string name, std::string algorithm,
                  ListSingletonContainer *lsc, Restraint *r) {
  double runtime;
  double value = 0;
  r->evaluate(false);
  IMP_TIME({
             IMP_CONTAINER_FOREACH(SingletonContainer, lsc, {
               XYZ(lsc->get_model(), _1).set_coordinates(get_random_vector_in(
                   BoundingBox3D(Vector3D(0, 0, 0), Vector3D(10, 10, 10))));
             });
             value += r->evaluate(false);
           },
           runtime);
  IMP::benchmark::report(name, algorithm, runtime, value);
}
}

int main(int argc, char **argv) {
  IMP::setup_from_argv(argc, argv, "Benchmark connectivity");
  unsigned int npart = 1000;
  set_log_level(SILENT);
  // set_check_level(NONE);
  IMP_NEW(Model, m, ());
  IMP_NEW(HarmonicUpperBoundSphereDistancePairScore, ss, (0, 1));
  ParticlesTemp ps = create_xyzr_particles(m, npart, .1);
  IMP_NEW(ListSingletonContainer, lsc, (m, IMP::internal::get_index(ps)));
  {
    IMP_NEW(ConnectivityRestraint, r, (ss, lsc));
    benchmark_it("connectivity", "slow", lsc, r);
  }
  {
    IMP_NEW(MSConnectivityRestraint, r, (m, ss));
    Ints composite;
    for (unsigned int i = 0; i < ps.size(); ++i) {
      composite.push_back(r->add_type(ParticlesTemp(1, ps[i])));
    }
    r->add_composite(composite);
    benchmark_it("ms_connectivity", "slow", lsc, r);
  }
  {
    IMP_NEW(ConnectingPairContainer, cpc, (lsc, .1));
    Pointer<Restraint> pr(
        container::create_restraint(ss.get(), cpc.get()));
    benchmark_it("connectivity", "fast", lsc, pr);
  }
  return IMP::benchmark::get_return_value();
}
