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
void do_benchmark(Model * m) {
 // measure time
  double runtime;
  double total=0;
  std::string fname= IMP::benchmark::get_data_path("large_protein.pdb");
  IMP_TIME(
             {
               atom::Hierarchy h= read_pdb(fname, m);
               total+=reinterpret_cast<size_t>(h.get_particle());
               atom::destroy(h);
             }, runtime);
  IMP::benchmark::report(std::string("pdb"), runtime, total);
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark pdb reading");
  IMP_NEW(Model, m, ());
  do_benchmark(m);
  return IMP::benchmark::get_return_value();
}
