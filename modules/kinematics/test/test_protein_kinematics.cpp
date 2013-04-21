/**
   This is the program for creating a simple kinematic tree

*/
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/XYZ.h>

#include <IMP/kinematics/ProteinKinematics.h>
#include <string>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

using namespace IMP::kinematics;

int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " pdb " << std::endl;
    exit(1);
  }
  std::string fname(argv[1]);
  std::cout << fname << std::endl;

  // read in the input protein
  IMP::Model *model = new IMP::Model();
  std::cerr << "Starting reading pdb file " << fname << std::endl;
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb
    (fname, model,
     new IMP::atom::NonWaterNonHydrogenPDBSelector(),
     //new IMP::atom::ATOMPDBSelector(),
     // don't add radii
     true, true);
  const std::string topology_file_name = "top_heav.lib";
  const std::string parameter_file_name = "par.lib";
  std::ifstream test(topology_file_name.c_str());
  if(!test) {
    std::cerr << "Please provide topology file " << topology_file_name
              << std::endl;
    exit(1);
  }
  std::ifstream test1(parameter_file_name.c_str());
  if(!test1) {
    std::cerr << "Please provide parameter file " << parameter_file_name
              << std::endl;
    exit(1);
  }

  IMP::atom::CHARMMParameters* ff =
    new IMP::atom::CHARMMParameters(topology_file_name, parameter_file_name);
  IMP::atom::CHARMMTopology* topology = ff->create_topology(mhd);
  //topology->apply_default_patches();
  topology->setup_hierarchy(mhd);
  //IMP::atom::CHARMMStereochemistryRestraint* r
  //   = new IMP::atom::CHARMMStereochemistryRestraint(mhd, topology);
  IMP::ParticlesTemp bonds =  topology->add_bonds(mhd);

  ProteinKinematics pk(mhd, true, false);
  std::cerr << "ProteinKinematics done" << std::endl;

  IMP::Particles residue_particles
    = IMP::atom::get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

  std::cerr << "Psi of the first residue "
            << pk.get_psi(IMP::atom::Residue(residue_particles[0]))
            << std::endl;
  pk.set_psi(IMP::atom::Residue(residue_particles[0]), 3.14);

  std::cerr << "Psi of the first residue after change"
            << pk.get_psi(IMP::atom::Residue(residue_particles[0]))
            << std::endl;

  IMP::atom::write_pdb(mhd, "./after_set_psi1_to_180deg.pdb");



  return 0;
}
