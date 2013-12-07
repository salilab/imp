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
    IMP_NEW(ComplementarityRestraint, r,
            (atom::get_leaves(h0), atom::get_leaves(h1)));
    r->set_maximum_penetration_score(10);
    r->set_maximum_separation(10);
    r->set_complementarity_value(0);
    r->set_complementarity_thickness(0);
    r->set_interior_layer_thickness(4);
    test_one<ComplementarityRestraint>(
        "wev3", seed, m, r->create_scoring_function(), rb0, false);
    test_one<ComplementarityRestraint>("wev3", seed, m,
                                       r->create_scoring_function(), rb0, true);
  }
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark methods for excluded volume");
  IMP_CATCH_AND_TERMINATE(do_it(););
  return IMP::benchmark::get_return_value();
}
