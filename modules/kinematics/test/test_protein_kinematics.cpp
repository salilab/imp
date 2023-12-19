/**
   This is the program for creating a simple kinematic tree

*/
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/XYZ.h>
#include <IMP/flags.h>

#include <IMP/kinematics/ProteinKinematics.h>
#include <string>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

using namespace IMP::kinematics;

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test protein kinematics.");

  std::string fname = IMP::kinematics::get_example_path("antibody/1igt.pdb");

  // read in the input protein
  IMP::Model* model = new IMP::Model();
//std::cerr << "Starting reading pdb file " << fname << std::endl;
  IMP::Strings chains;
  chains.push_back("A");
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      fname, model, new IMP::atom::ChainPDBSelector(chains),
      // new IMP::atom::ATOMPDBSelector(),
      // don't add radii
      true, true);

  /* Speed up test by only reading 10 residues from the A chain */
  assert(mhd.get_number_of_children() == 1);
  IMP::atom::Hierarchy chain = mhd.get_child(0);
  for (unsigned int i = chain.get_number_of_children() - 1; i >= 10; --i) {
    chain.remove_child(i);
  }

  const std::string topology_file_name = IMP::atom::get_data_path("top_heav.lib");
  const std::string parameter_file_name = IMP::atom::get_data_path("par.lib");
  std::ifstream test(topology_file_name.c_str());
  if (!test) {
    std::cerr << "Please provide topology file " << topology_file_name
              << std::endl;
    exit(1);
  }
  std::ifstream test1(parameter_file_name.c_str());
  if (!test1) {
    std::cerr << "Please provide parameter file " << parameter_file_name
              << std::endl;
    exit(1);
  }

  IMP::atom::CHARMMParameters* ff =
      new IMP::atom::CHARMMParameters(topology_file_name, parameter_file_name);
  IMP::atom::CHARMMTopology* topology = ff->create_topology(mhd);
  // topology->apply_default_patches();
  topology->setup_hierarchy(mhd);
  // IMP::atom::CHARMMStereochemistryRestraint* r
  //   = new IMP::atom::CHARMMStereochemistryRestraint(mhd, topology);
  IMP::ParticlesTemp bonds = topology->add_bonds(mhd);

  IMP_NEW(ProteinKinematics, pk, (mhd, true, false));
//std::cerr << "ProteinKinematics done" << std::endl;

  IMP::Particles residue_particles =
      IMP::atom::get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

/*std::cerr << "Psi of the first residue "
            << pk->get_psi(IMP::atom::Residue(residue_particles[0]))
            << std::endl;*/
  pk->set_psi(IMP::atom::Residue(residue_particles[0]), 3.14);

/*std::cerr << "Psi of the first residue after change"
            << pk->get_psi(IMP::atom::Residue(residue_particles[0]))
            << std::endl;*/

//IMP::atom::write_pdb(mhd, "./after_set_psi1_to_180deg.pdb");

  return 0;
}
