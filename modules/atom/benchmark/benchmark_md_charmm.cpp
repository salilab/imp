/**
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/display.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::display;

namespace {
int do_benchmark() {
  try {
    IMP_NEW(Model, m, ());
    atom::Hierarchy prot =
        read_pdb(IMP::benchmark::get_data_path("extended.pdb"), m);
    // Read in the CHARMM heavy atom topology and parameter files

    IMP_NEW(CHARMMParameters, ff,
            (IMP::benchmark::get_data_path("toph19.inp"),
             IMP::benchmark::get_data_path("param19.inp")));
    /*Pointer<CHARMMParameters> ff
      = new CHARMMParameters("toph19.inp", "param19.inp"));*/

    // Using the CHARMM libraries, determine the ideal topology (atoms and their
    // connectivity) for the PDB file's primary sequence
    Pointer<CHARMMTopology> topology = ff->create_topology(prot);

    // Typically this modifies the C and N termini of each chain in the protein
    // by
    // applying the CHARMM CTER and NTER patches. Patches can also be manually
    // applied at this point, e.g. to add disulfide bridges.
    topology->apply_default_patches();

    /* Make the PDB file conform with the topology; i.e. if it contains extra
       atoms that are not in the CHARMM topology file, remove them; if it is
       missing atoms (e.g. sidechains, hydrogens) that are in the CHARMM
       topology,
       add them and construct their Cartesian coordinates from internal
       coordinate
       information.*/
    topology->setup_hierarchy(prot);

    IMP_NEW(RestraintSet, rs, (m, "All restraints"));
    // Set up and evaluate the stereochemical part (bonds, angles, dihedrals,
    // impropers) of the CHARMM forcefield
    IMP_NEW(CHARMMStereochemistryRestraint, r, (prot, topology));
    rs->add_restraint(r);

    /* Add non-bonded interaction (in this case, Lennard-Jones). This needs to
       know the radii and well depths for each atom, so add them from the
       forcefield (they can also be assigned manually using the XYZR or
       LennardJones decorators): */
    ff->add_radii(prot);
    ff->add_well_depths(prot);

    // Get a list of all atoms in the protein, and put it in a container
    atom::Hierarchies atoms = get_by_type(prot, ATOM_TYPE);
    // work around added atoms not being marked as optimized
    IMP_FOREACH(atom::Hierarchy atom, atoms) {
      core::XYZ(atom).set_coordinates_are_optimized(true);
    }
    IMP_NEW(ListSingletonContainer, cont, (m, IMP::internal::get_index(atoms)));

    /* Add a restraint for the Lennard-Jones interaction. This is built from
       a collection of building blocks. First, a
       ClosePairContainer maintains a list
       of all pairs of Particles that are close.
       Next, all 1-2, 1-3 and 1-4 pairs
       from the stereochemistry created above are filtered out.
       Then, a LennardJonesPairScore scores a pair of atoms with the
       Lennard-Jones
       potential. Finally, a PairsRestraint is used which simply applies the
       LennardJonesPairScore to each pair in the ClosePairContainer. */
    IMP_NEW(ClosePairContainer, nbl, (cont, 4.0));
    nbl->add_pair_filter(r->get_pair_filter());

    IMP_NEW(ForceSwitch, sf, (6.0, 7.0));
    IMP_NEW(LennardJonesPairScore, ps, (sf));
    rs->add_restraint(new PairsRestraint(ps, nbl));

    // Finally, evaluate the score of the whole system (without derivatives)
    IMP_NEW(ConjugateGradients, cg, (m));
    cg->set_scoring_function(rs);
    if (IMP::run_quick_test) {
      cg->optimize(1);
    } else {
      cg->optimize(1000);
    }

    //## Molecular Dynamics
    IMP_NEW(MolecularDynamics, md, (m));
    md->set_scoring_function(rs);
    md->assign_velocities(300);
    md->set_maximum_time_step(2.0);
    /*## therm legend
      # 0 :: nve
      # 1 :: rescale velocities
      # 2 :: berendsen
      # 3 :: langevin
      #md.set_therm(0,0,0)
      #md.set_therm(1,300,0)
      #md.set_therm(2,300,100)
      #md.set_therm(3,300,0.01)

      # metadynamics setup
      #md.mtd_setup(0.003, 10.0, -200.0, 400.0)

      # GO! */
    IMP_NEW(atom::RemoveRigidMotionOptimizerState, rmos,
            (m, md->get_simulation_particle_indexes()));
    rmos->set_period(10);
    md->add_optimizer_state(rmos);
    IMP_NEW(atom::LangevinThermostatOptimizerState, therm,
            (m, md->get_simulation_particle_indexes(), 300, 500));
    md->add_optimizer_state(therm);
    double time, score = 0;
    if (IMP::run_quick_test) {
      time = 0;
      score += md->optimize(2);
    } else {
      IMP_TIME({ score += md->optimize(100); }, time);
    }
    IMP::benchmark::report("md charmm", time, score);
    return 0;
  }
  catch (const Exception &e) {
    std::cerr << "Exception " << e.what() << std::endl;
    return 1;
  }
}
}

int main(int argc, char **argv) {
  IMP::setup_from_argv(argc, argv, "Benchmark md");
  IMP::set_log_level(IMP::SILENT);
  return do_benchmark();
}
