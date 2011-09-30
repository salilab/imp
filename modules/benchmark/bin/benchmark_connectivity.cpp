/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
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

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::container;

namespace {
void benchmark_it(std::string name, ListSingletonContainer *lsc, Model *m) {
  double runtime;
  double value=0;
  m->evaluate(false);
  IMP_TIME({
      unsigned int np= lsc->get_number_of_particles();
      for (unsigned int i=0; i< np; ++i) {
        XYZ(lsc->get_particle(i))
          .set_coordinates(get_random_vector_in(BoundingBox3D(Vector3D(0,0,0),
                                                        Vector3D(10,10,10))));
      }
      value+= m->evaluate(false);
    }, runtime);
  IMP::benchmark::report(name, runtime, value);
}
}

int main() {
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
    benchmark_it("connectivity slow", lsc, m);
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
    benchmark_it("connectivity slow", lsc, m);
    m->remove_restraint(r);
  }
  {
    IMP_NEW(ConnectingPairContainer, cpc,(lsc, .1));
    Pointer<Restraint> pr(create_restraint(ss, cpc));
    m->add_restraint(pr);
    benchmark_it("connectivity fast", lsc, m);
    m->remove_restraint(pr);
  }
  return IMP::benchmark::get_return_value();
}
