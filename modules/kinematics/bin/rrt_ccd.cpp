/**
   This is the program for creating a simple kinematic tree from a protein
   and running rrt on phi psi angle
*/
#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <IMP/core/SphereDistancePairScore.h>

#include <IMP/atom/pdb.h>
#include <IMP/atom/dihedrals.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

#include <IMP/container/generic.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/PairsRestraint.h>

#include <IMP/kinematics/helpers.h>
#include <IMP/kinematics/ProteinKinematics.h>
#include <IMP/kinematics/RRT.h>
#include <IMP/kinematics/DOF.h>
#include <IMP/kinematics/local_planners.h>
#include <IMP/kinematics/directional_DOFs.h>
#include <IMP/kinematics/UniformBackboneSampler.h>
#include <IMP/kinematics/KinematicForestScoreState.h>
#include <IMP/kinematics/CCDLoopClosure.h>

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::kinematics;

int main(int argc, char **argv)
{
  // output arguments
  for (int i=0; i<argc; i++) {
    std::cerr << argv[i] << " ";
  }
  std::cerr << std::endl;

  int number_of_iterations = 100;
  int number_of_nodes = 100;
  int save_configuration_number = 10;
  int number_of_models_in_pdb = 100;
  int number_of_active_dofs = 0;
  float radii_scaling = 0.5;
  bool reset_angles = false;
  std::string connect_chains_file;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "PDB file and rotatable angles file")
    ("version", "Written by Dina Schneidman.")
    ("number_of_iterations,i", po::value<int>(&number_of_iterations)->default_value(100),
     "number of iterations")
    ("number_of_nodes,n", po::value<int>(&number_of_nodes)->default_value(100),
     "number of nodes")
    ("number_of_path_configurations_saved,p", po::value<int>(&save_configuration_number)->default_value(10),
     "if the path between two nodes is feasible, each Nth configuration on the path will be added to the tree")
    ("number_of_active_dofs,a", po::value<int>(&number_of_active_dofs)->default_value(0),
     "for many dofs use this option with 10-50 dofs")
    ("radii_scaling,s", po::value<float>(&radii_scaling)->default_value(0.5, "0.5"),
     "radii scaling parameter (0.3 < s < 1.0)")
    ("reset_angles", "set initial values in rotatable angles to PI (default = false)")
    ("connect_chains_file,c", po::value<std::string>(&connect_chains_file),
     "connect rigid bodies from different chains into one rigid body by adding bonds between specified atoms or residues")
    ("number_of_models_in_pdb,m", po::value<int>(&number_of_models_in_pdb)->default_value(100),
     "number of models in output PDB files")
    ;

  float angle_range = IMP::algebra::PI;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and rotatable angles files")
    ("angle_range,r", po::value<float>(&angle_range)->default_value(IMP::algebra::PI),
     "angle range for sampling, (-angle < sampled_angle < +angle, default = PI)")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::options_description visible("Usage: <pdb_file> <rotatable_angles_file>");
  visible.add(desc);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc,
            argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << visible << "\n";
    return 0;
  }
  if(radii_scaling < 0.5 || radii_scaling > 1.0) {
    std::cerr << "radii_scaling parameter outside allowed range "
	      << radii_scaling << std::endl;
  }
  if(vm.count("reset_angles")) reset_angles=true;

  std::string pdb_name = files[0];
  std::string angle_file_name;
  if(files.size() > 1) angle_file_name = files[1];

  // read in the input protein
  IMP::Pointer<IMP::Model> model = new IMP::Model();
  std::cerr << "Starting reading pdb file " << pdb_name << std::endl;
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(pdb_name, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                        // don't add radii
                        true, true);

  IMP::ParticlesTemp atoms = IMP::atom::get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::ParticlesTemp residues = IMP::atom::get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

  const std::string topology_file_name = IMP::atom::get_data_path("top_heav.lib");
  const std::string parameter_file_name = IMP::atom::get_data_path("par.lib");
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
  IMP::Pointer<IMP::atom::CHARMMTopology> topology =
    ff->create_topology(mhd);

  // We don't want to add/remove any atoms, so we only add atom types
  topology->apply_default_patches();
  //  topology->setup_hierarchy(mhd); //removes atoms
  topology->add_atom_types(mhd);

  IMP::ParticlesTemp bonds =  topology->add_bonds(mhd);

  // create phi/psi joints
  IMP::atom::Residues flexible_residues;
  std::vector<IMP::atom::Atoms> dihedral_angles;
  if(angle_file_name.length()>0) {
    read_angle_file(angle_file_name, residues, atoms,
                    flexible_residues, dihedral_angles);
  } else {
    flexible_residues = residues;
  }

  std::vector<IMP::atom::Atoms> connect_chains_atoms;
  if(connect_chains_file.length() > 0) {
    read_connect_chains_file(connect_chains_file, atoms, connect_chains_atoms);
    for(unsigned i=0; i<connect_chains_atoms.size(); i++) {
      IMP::atom::Bond bond = create_bond(connect_chains_atoms[i]);
      if(bond != IMP::atom::Bond()) bonds.push_back(bond);
    }
  }

  IMP::ParticlesTemp angles = ff->create_angles(bonds);
  IMP::ParticlesTemp dihedrals = ff->create_dihedrals(bonds);
  std::cerr << "# atoms " << atoms.size()
            << " # bonds " << bonds.size()
            << " # angles " << angles.size()
            << " # dihedrals " << dihedrals.size() << std::endl;

  add_missing_bonds(atoms, bonds);

  std::cerr << "# atoms " << atoms.size()
            << " # bonds " << bonds.size()
            << " # angles " << angles.size()
            << " # dihedrals " << dihedrals.size() << std::endl;

  // add radius
  ff->add_radii(mhd, radii_scaling);

  // prepare exclusions list
  IMP_NEW(IMP::atom::StereochemistryPairFilter, pair_filter, ());
  pair_filter->set_bonds(bonds);
  pair_filter->set_angles(angles);
  pair_filter->set_dihedrals(dihedrals);

  // close pair container
  IMP_NEW(IMP::container::ListSingletonContainer, lsc,
                                (model, IMP::get_indexes(atoms)));
  IMP_NEW(IMP::container::ClosePairContainer, cpc, (lsc, 15.0));
  cpc->add_pair_filter(pair_filter);

  IMP_NEW(IMP::core::SoftSpherePairScore,  score,(1));
  //IMP_NEW(IMP::container::PairsRestraint, pr, (score, cpc));

  IMP::core::SoftSpherePairScore* score_ptr = score.release();
  IMP::container::ClosePairContainer* cpc_ptr = cpc.release();
  IMP::Pointer<IMP::Restraint> pr=
    IMP::container::create_restraint(score_ptr, cpc_ptr, "stereochemistry");

  // where we open the loop
  IMP::atom::Atoms open_loop_bond_atoms;
  IMP::atom::Atoms psi_atoms =
    IMP::atom::get_psi_dihedral_atoms(flexible_residues.back());
  open_loop_bond_atoms.push_back(psi_atoms[2]);
  open_loop_bond_atoms.push_back(psi_atoms[3]);

  IMP_NEW(ProteinKinematics, pk,
          (mhd, flexible_residues, dihedral_angles, open_loop_bond_atoms));
  std::cerr << "ProteinKinematics done" << std::endl;
  DihedralAngleRevoluteJoints joints = pk->get_ordered_joints();
  IMP_NEW(KinematicForestScoreState, kfss, (pk->get_kinematic_forest(),
                                            pk->get_rigid_bodies(),
                                            atoms));
  model->add_score_state(kfss);

  // create dofs
  DOFs dofs;
  for(unsigned int i=0; i<joints.size(); i++) {
    std::cerr << "Angle = " << joints[i]->get_angle() << " " << 180 * joints[i]->get_angle()/IMP::algebra::PI << std::endl;
    if(reset_angles) joints[i]->set_angle(IMP::algebra::PI);
    /*IMP_NEW(DOF, dof, (joints[i]->get_angle(),
                       -IMP::algebra::PI,
                       IMP::algebra::PI,
                       IMP::algebra::PI/360));
    */
    double angle = joints[i]->get_angle();
    IMP_NEW(DOF, dof, (angle, angle-angle_range, angle+angle_range, IMP::algebra::PI/360));
    dofs.push_back(dof);
  }

  DOFValues val(dofs);

  if(reset_angles) {
    kfss->do_before_evaluate();
    for(unsigned int i=0; i<joints.size(); i++) {
      std::cerr << "Angle = " << joints[i]->get_angle() << " " << 180 * joints[i]->get_angle()/IMP::algebra::PI << std::endl;
    }
  }

  DirectionalDOF dd(dofs);

  IMP_NEW(UniformBackboneSampler, ub_sampler, (joints, dofs));
  IMP_NEW(PathLocalPlanner, planner, (model, ub_sampler, &dd,
                                      save_configuration_number));

  std::cerr << "Init  RRT" << std::endl;
  IMP_NEW(RRT, rrt, (model, ub_sampler, planner, dofs, number_of_iterations,
                     number_of_nodes, number_of_active_dofs));
  rrt->set_scoring_function(pr);

  DihedralAngleRevoluteJoints loop_joints = pk->get_loop_joints();
  std::cerr << "All joints = " << joints.size() << " loop joints = " << loop_joints.size() << std::endl;

  CCDLoopClosure* loop_closer;
  IMP::core::XYZ M1 = loop_joints[1]->get_b();
  IMP::core::XYZ M2 = loop_joints[0]->get_b();
  IMP::core::XYZ M3 = loop_joints[0]->get_c();
  IMP::algebra::Vector3D F1 = M1.get_coordinates();
  IMP::algebra::Vector3D F2 = M2.get_coordinates();
  IMP::algebra::Vector3D F3 = M3.get_coordinates();
  loop_closer = new CCDLoopClosure(loop_joints, F1, F2, F3, M1, M2, M3);

  std::string filename = "node_begin.pdb";
  IMP::atom::write_pdb(mhd, filename);

  if(number_of_iterations == 0) { //just test loop closure
    // change values to open loop
    for(unsigned int i=1; i<loop_joints.size(); i++) {
      loop_joints[i]->set_angle(loop_joints[i]->get_angle()+30.0);
      loop_joints[i]->get_owner_kf()->update_all_external_coordinates();
    }
    if(loop_closer->close_loop()) {
      // check collisions
      double score = rrt->get_scoring_function()->evaluate_if_below(false, 0.0);
      if(score <= 0.0001) {
        IMP::atom::write_pdb(mhd, "closed_loop.pdb");
        std::cerr << "collision free loop " << std::endl;
      } else {
        std::cerr << "collision found in closed loop " << score << std::endl;
      }
    } else {
      std::cerr << "Couldn't close loop " << std::endl;
    }
  }

  std::cerr << "Start RRT run" << std::endl;
  // rrt->run();
  // std::cerr << "Done RRT " << rrt->get_DOFValues().size() << std::endl;

  // output PDBs and close loops
  std::ofstream *out = NULL;
  std::ofstream *out2 = NULL;
  int file_counter = 1;
  int model_counter = 0;
  int last_model_written = 0;

  while(rrt->run(100)) { // output every 100 iterations
    std::vector<DOFValues> dof_values = rrt->get_DOFValues();
    for(unsigned int i = last_model_written; i<dof_values.size(); i++) {
      ub_sampler->apply(dof_values[i]);
      kfss->do_before_evaluate();

      // open new file if needed
      if(model_counter % number_of_models_in_pdb == 0) { // open new file
        if(out !=NULL) out->close();
        if(out2 !=NULL) out2->close();
        std::string file_name = "nodes" + std::string(boost::lexical_cast<std::string>(file_counter)) + ".pdb";
        std::string file_name2 = "nodes_closed" + std::string(boost::lexical_cast<std::string>(file_counter)) + ".pdb";
        out = new std::ofstream(file_name.c_str());
        out2 = new std::ofstream(file_name2.c_str());
        file_counter++;
        model_counter=0;
      }

      IMP::atom::write_pdb(mhd, *out, model_counter+1);
      if(loop_closer->close_loop()) {
        // check collisions
        double score = rrt->get_scoring_function()->evaluate_if_below(false, 0.0);
        if(score <= 0.0001) {
          IMP::atom::write_pdb(mhd, *out2, model_counter+1);
          std::cerr << "collision free loop " << model_counter+1 << std::endl;
        } else {
          std::cerr << "collision found in closed loop " << score << std::endl;
        }
      } else {
        std::cerr << "Couldn't close loop " << model_counter+1 << std::endl;
      }
      model_counter++;
    }
    last_model_written = dof_values.size() - 1;
  }

  std::vector<DOFValues> dof_values = rrt->get_DOFValues();
  std::cerr << "Done RRT " << dof_values.size() << std::endl;

  // write remaining nodes
  for(unsigned int i = last_model_written; i<dof_values.size(); i++) {
    ub_sampler->apply(dof_values[i]);
    kfss->do_before_evaluate();

    // open new file if needed
    if(model_counter % number_of_models_in_pdb == 0) { // open new file
      if(out !=NULL) out->close();
      if(out2 !=NULL) out2->close();
      std::string file_name = "nodes" + std::string(boost::lexical_cast<std::string>(file_counter)) + ".pdb";
      std::string file_name2 = "nodes_closed" + std::string(boost::lexical_cast<std::string>(file_counter)) + ".pdb";
      out = new std::ofstream(file_name.c_str());
      out2 = new std::ofstream(file_name2.c_str());
      file_counter++;
      model_counter=0;
    }

    IMP::atom::write_pdb(mhd, *out, model_counter+1);
    if(loop_closer->close_loop()) {
      // check collisions
      double score = rrt->get_scoring_function()->evaluate_if_below(false, 0.0);
      if(score <= 0.0001) {
        IMP::atom::write_pdb(mhd, *out2, model_counter+1);
        std::cerr << "collision free loop " << model_counter+1 << std::endl;
      } else {
        std::cerr << "collision found in closed loop " << score << std::endl;
      }
    } else {
      std::cerr << "Couldn't close loop " << model_counter+1 << std::endl;
    }
    model_counter++;
  }
  out->close();
  out2->close();
  return 0;
}
