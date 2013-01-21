/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>

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

namespace {
template <class PT>
void do_benchmark(std::string name, Model *,
                  const PT &ps, Refiner *r, double ) {
 // measure time
  double runtime;
  double total=0;
  IMP_TIME(
             {
               for (unsigned int i=0; i< ps.size(); ++i) {
                 ParticlesTemp nps= r->get_refined(ps[i]);
                 total+=nps.size();
               }
             }, runtime);
  IMP::benchmark::report(std::string("refiner ")+name, runtime, total);
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark refiners");
  IMP_NEW(Model, m, ());
  IMP_NEW(LeavesRefiner, lr, (atom::Hierarchy::get_traits()));
  lr->set_was_used(true);
  atom::Hierarchies hs;
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
      ParticlesTemp lps= lr->get_refined(hs[i]);
      for (unsigned int i=0; i< lps.size(); ++i) {
        h0.add_child(core::Hierarchy::setup_particle(lps[i], tr));
      }
    }
    IMP_NEW(ChildrenRefiner, cr, (tr));
    cr->set_was_used(true);
    do_benchmark("children", m, ps, cr, 0.416667);
  }
  {
    do_benchmark("leaves", m, hs, lr, 0.090692);
  }
  return IMP::benchmark::get_return_value();
}
