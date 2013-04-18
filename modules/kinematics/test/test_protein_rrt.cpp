/**
   This is the program for creating a simple kinematic tree from a protein
   and running rrt on phi psi angle
*/
#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <IMP/algebra/Vector3D.h>
#include <IMP/container/generic.h>

#include <IMP/core/XYZR.h>
#include <IMP/core/SphereDistancePairScore.h>

#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/PairsRestraint.h>

#include <IMP/kinematics/ProteinKinematics.h>
#include <IMP/kinematics/RRT.h>
#include <IMP/kinematics/DOF.h>
#include <IMP/kinematics/local_planners.h>
#include <IMP/kinematics/directional_DOFs.h>
#include <IMP/kinematics/UniformBackboneSampler.h>
#include <IMP/kinematics/KinematicForestScoreState.h>

#include <string>
#include <boost/lexical_cast.hpp>

void scale_radii(IMP::ParticlesTemp& particles, double scale) {
  for(unsigned int i=0; i<particles.size(); i++) {
    IMP::core::XYZR xyzr(particles[i]);
    xyzr.set_radius(xyzr.get_radius()*scale);
  }
}

using namespace IMP::kinematics;

int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  if(argc != 2 && argc != 3) {
    std::cerr << "Usage: " << argv[0] << " pdb [radii_scale]" << std::endl;
    exit(1);
  }
  std::string fname(argv[1]);
  std::cout << fname << std::endl;
  double scale = 0.7;
  if(argc == 3) scale = atof(argv[2]);

  // read in the input protein
  IMP::Pointer<IMP::Model> model = new IMP::Model();
  std::cerr << "Starting reading pdb file " << fname << std::endl;
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(fname, model,
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

  IMP::atom::CHARMMParameters*
    ff =  new IMP::atom::CHARMMParameters
    (topology_file_name, parameter_file_name);
  IMP::Pointer<IMP::atom::CHARMMTopology> topology = ff->create_topology(mhd);
  //topology->apply_default_patches();
  topology->setup_hierarchy(mhd);
  //IMP::atom::CHARMMStereochemistryRestraint* r =
  //  new IMP::atom::CHARMMStereochemistryRestraint(mhd, topology);


  IMP::ParticlesTemp atoms = IMP::atom::get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::ParticlesTemp bonds =  topology->add_bonds(mhd);
  IMP::ParticlesTemp angles = ff->create_angles(bonds);
  IMP::ParticlesTemp dihedrals = ff->create_dihedrals(bonds);
  std::cerr << "# bonds " << bonds.size()
            << " # angles " << angles.size()
            << " # dihedrals " << dihedrals.size() << std::endl;

  // add radius
  ff->add_radii(mhd);
  scale_radii(atoms, scale);

  // prepare exclusions list
  IMP_NEW(IMP::atom::StereochemistryPairFilter, pair_filter, ());
  pair_filter->set_bonds(bonds);
  pair_filter->set_angles(angles);
  pair_filter->set_dihedrals(dihedrals);

  // close pair container
  IMP_NEW(IMP::container::ListSingletonContainer, lsc, (atoms));
  IMP_NEW(IMP::container::ClosePairContainer, cpc, (lsc, 15.0));
  cpc->add_pair_filter(pair_filter);

  IMP_NEW(IMP::core::SoftSpherePairScore,  score,(1));
  IMP_NEW(IMP::container::PairsRestraint, pr, (score, cpc));

  // TODO: check why not working: should be much faster
  //IMP::base::Pointer<IMP::Restraint> pr=
  //   IMP::container::create_restraint(score, cpc);
  model->add_restraint(pr);

  // create phi/psi joints
  ProteinKinematics pk(mhd, true, false);
  std::cerr << "ProteinKinematics done" << std::endl;
  DihedralAngleRevoluteJoints joints = pk.get_joints();
  IMP_NEW(KinematicForestScoreState, kfss, (pk.get_kinematic_forest(),
                                            pk.get_rigid_bodies(),
                                            atoms));
  model->add_score_state(kfss);

  // create dofs
  DOFs dofs;
  for(unsigned int i=0; i<joints.size(); i++) {
    std::cerr << "Angle = " << joints[i]->get_angle() << std::endl;
    IMP_NEW(DOF, dof, (joints[i]->get_angle(),
                       -IMP::algebra::PI,
                       IMP::algebra::PI,
                       IMP::algebra::PI/360));
    dofs.push_back(dof);
  }
  UniformBackboneSampler sampler(joints, dofs);
  DOFValues val(dofs);
  std::cerr << "DOFs done" << std::endl;

  DirectionalDOF dd(dofs);
  PathLocalPlanner planner(model, &sampler, &dd, 10);
  std::cerr << "Start RRT" << std::endl;
  IMP_NEW(RRT, rrt, (model, &sampler, &planner, dofs));
  std::cerr << "Start RRT run" << std::endl;
  rrt->run();
  std::cerr << "Done RRT" << std::endl;

  // output PDBs
  std::vector<DOFValues> dof_values = rrt->get_DOFValues();
  for(unsigned int i = 0; i<dof_values.size(); i++) {
    sampler.apply(dof_values[i]);
    kfss->do_before_evaluate();
    std::string filename = "node"
      + std::string(boost::lexical_cast<std::string>(i+1)) + ".pdb";
    IMP::atom::write_pdb(mhd, filename);
  }
  return 0;
}
