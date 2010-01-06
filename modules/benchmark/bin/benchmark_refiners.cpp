/**
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/macros.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;

#define N 1

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

template <class PT>
void do_benchmark(std::string name, Model *m,
                  const PT &ps, Refiner *r) {
 // measure time
  double runtime;
  double total=0;
  IMP_TIME_N(
             {
               for (unsigned int i=0; i< ps.size(); ++i) {
                 ParticlesTemp nps= r->get_refined(ps[i]);
                 total+=nps.size();
               }
             }, runtime, N);
  IMP::benchmark::report(name+" refiner", runtime, total);
}


int main(int argc, char **argv) {
  IMP_NEW(Model, m, ());
  IMP_NEW(LeavesRefiner, lr, (atom::Hierarchy::get_traits()));
  atom::HierarchiesTemp hs;
  for (unsigned int i=0; i< 10; ++i) {
    hs.push_back(
            read_pdb(benchmark::get_data_path("huge_protein.pdb"), m));
  }

  {
    ParticlesTemp ps;
    core::HierarchyTraits tr=core::Hierarchy::get_default_traits();
    for (unsigned int i=0; i< hs.size(); ++i) {
      IMP_NEW(Particle, p, (m));
      ps.push_back(p);
      core::Hierarchy h0= core::Hierarchy::setup_particle(p,tr);
      Particles ps= lr->get_refined(hs[0]);
      for (unsigned int i=0; i< ps.size(); ++i) {
        h0.add_child(core::Hierarchy::setup_particle(ps[i], tr));
      }
    }
    IMP_NEW(ChildrenRefiner, cr, (tr));
    do_benchmark("children", m, ps, cr);
  }
  {
    do_benchmark("leaves", m, hs, lr);
  }
  return 0;
}
