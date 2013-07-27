/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include "benchmark_excluded_volume.h"

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::benchmark;
using namespace IMP::container;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::multifit;

namespace {

void do_it() {
 IMP_EV_BENCHMARK_SETUP

  {
    IMP_NEW(ClosePairContainer, cpc, (lsc, 0, 5));
    IMP_NEW(SoftSpherePairScore, ps, (1));
    base::Pointer<Restraint>
      sr(container::create_restraint(ps.get(), cpc.get()));
    sr->set_maximum_score(.1);
    test_one<ClosePairContainer>("pairs restraint", seed, m,
                                 sr->create_scoring_function(), rb0,
                                 false);
    test_one<ClosePairContainer>("pairs restraint", seed, m,
                                 sr->create_scoring_function(), rb0,
                                 true);
  }
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark methods for excluded volume");
  IMP_CATCH_AND_TERMINATE(do_it(););
  return IMP::benchmark::get_return_value();
}
