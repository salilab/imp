/**
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
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

void trim_chain(atom::Hierarchy h) {
  atom::Hierarchies cs = atom::get_by_type(h, atom::CHAIN_TYPE);
  if (cs.size() == 1) {
    while (cs[0].get_number_of_children() > 40) {
      cs[0].remove_child(40);
    }
  }
}

void do_it() {
  IMP_NEW(kernel::Model, m, ());
  int seed = IMP::base::random_number_generator();
  atom::Hierarchy h0 =
      read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m);
  atom::Hierarchy h1 =
      read_pdb(IMP::benchmark::get_data_path("small_protein.pdb"), m);
  // Cut proteins down to 50 residues if a quick test is desired
  if (IMP::base::run_quick_test) {
    trim_chain(h0);
    trim_chain(h1);
  }

  RigidBody rb0 = create_rigid_body(h0);
  RigidBody rb1 = create_rigid_body(h1);
  rb0.set_coordinates(IMP::algebra::Vector3D(0, 0, 0));
  rb1.set_coordinates(IMP::algebra::Vector3D(0, 0, 0));
  kernel::ParticlesTemp leaves = get_leaves(h0);
  kernel::ParticlesTemp leaves1 = get_leaves(h1);
  leaves.insert(leaves.end(), leaves1.begin(), leaves1.end());
  IMP_NEW(ListSingletonContainer, lsc, (leaves));
  lsc->set_was_used(true);

  {

    IMP_NEW(ExcludedVolumeRestraint, evr, (lsc, 1, 5));
    evr->set_maximum_score(.1);
    test_one<ExcludedVolumeRestraint>(
        "excluded volume", seed, m, evr->create_scoring_function(), rb0, false);
    test_one<ExcludedVolumeRestraint>(
        "excluded volume", seed, m, evr->create_scoring_function(), rb0, true);
  }
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark methods for excluded volume");
  IMP_CATCH_AND_TERMINATE(do_it(););
  return IMP::benchmark::get_return_value();
}
