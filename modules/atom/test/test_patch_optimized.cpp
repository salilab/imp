/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/display.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>
#include <IMP/test/test_macros.h>


using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::display;

namespace {

const char *pdb =
  "ATOM      1  HT1 ARG     1       3.836  65.763  35.255  1.00  0.00  \n" \
  "ATOM      2  HT2 ARG     1       2.546  66.040  34.188  1.00  0.00  \n" \
  "ATOM      3  N   ARG     1       3.477  66.382  34.500  1.00 33.79  \n" \
  "ATOM      4  HT3 ARG     1       3.321  67.184  35.142  1.00  0.00  \n" \
  "ATOM      5  CA  ARG     1       4.292  66.713  33.342  1.00 31.17  \n" \
  "ATOM      6  CB  ARG     1       5.584  67.316  33.787  1.00 34.92  \n" \
  "ATOM      7  CG  ARG     1       6.365  67.976  32.693  1.00 38.41  \n" \
  "ATOM      8  CD  ARG     1       7.455  68.842  33.276  1.00 41.67  \n" \
  "ATOM      9  NE  ARG     1       8.358  69.342  32.236  1.00 43.96  \n" \
  "ATOM     10  HE  ARG     1       8.037  69.155  31.330  1.00  0.00  \n" \
  "ATOM     11  CZ  ARG     1       9.490  69.981  32.485  1.00 45.32  \n" \
  "ATOM     12  NH1 ARG     1       9.965  70.101  33.716  1.00 45.71  \n" \
  "ATOM     13 HH11 ARG     1       9.492  69.673  34.486  1.00  0.00  \n" \
  "ATOM     14 HH12 ARG     1      10.792  70.643  33.868  1.00  0.00  \n" \
  "ATOM     15  NH2 ARG     1      10.255  70.400  31.511  1.00 45.55  \n" \
  "ATOM     16 HH21 ARG     1       9.937  70.361  30.563  1.00  0.00  \n" \
  "ATOM     17 HH22 ARG     1      11.177  70.729  31.715  1.00  0.00  \n" \
  "ATOM     18  C   ARG     1       4.452  65.435  32.495  1.00 27.15  \n" \
  "ATOM     19  O   ARG     1       3.493  65.038  31.801  1.00 25.37  \n" \
  "ATOM     20  N   THR     2       5.607  64.782  32.604  1.00 22.75  \n" \
  "ATOM     21  H   THR     2       6.326  65.034  33.220  1.00  0.00  \n" \
  "ATOM     22  CA  THR     2       5.892  63.526  31.966  1.00 21.24  \n" \
  "ATOM     23  CB  THR     2       7.391  63.353  31.697  1.00 22.20  \n";


int do_benchmark() {
  try {
    IMP_NEW(kernel::Model, m, ());
    std::istringstream iss(pdb);
    atom::Hierarchy prot =  atom::read_pdb(iss, m);
    // Read in the CHARMM heavy atom topology and parameter files

    base::Pointer<CHARMMTopology> topology
      = get_all_atom_CHARMM_parameters()->create_topology(prot);

    topology->apply_default_patches();

    topology->setup_hierarchy(prot);
    atom::Hierarchies atoms = get_by_type(prot, ATOM_TYPE);

    //## Molecular Dynamics
    IMP_NEW(MolecularDynamics, md, (m));
    IMP::kernel::ParticleIndexes pis = md->get_simulation_particle_indexes();
    IMP_TEST_EQUAL(pis.size(), atoms.size());
    for (unsigned int i = 0; i < atoms.size(); ++i) {
      IMP_TEST_EQUAL(pis[i], atoms[i].get_particle_index());
    }
    return 0;
  } catch (IMP::base::Exception e) {
    std::cerr << "Caught exception " << e.what() << std::endl;
    return 1;
  }
}
}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv, "Test optimized patches");
  return do_benchmark();
}
