/**
   This is the program for creating a simple kinematic tree from a protein
   and running rrt on phi psi angle
*/
#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <IMP/core/SphereDistancePairScore.h>

#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

#include <IMP/container/generic.h>
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

#include <IMP/saxs/utility.h>

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

namespace {

IMP::atom::Atom find_atom(const IMP::ParticlesTemp& atoms, int input_index) {
  for(unsigned int i=0; i<atoms.size(); i++) {
    IMP::atom::Atom ad = IMP::atom::Atom(atoms[i]);
    if(ad.get_input_index() == input_index) return ad;
  }
  std::cerr << "Atom not found " << input_index << std::endl;
  exit(0);
}

IMP::atom::Residue find_residue(const IMP::ParticlesTemp& residues,
                                int res_index, std::string chain) {
  for(unsigned int i=0; i<residues.size(); i++) {
    IMP::atom::Residue rd = IMP::atom::Residue(residues[i]);
    if(rd.get_index() == res_index &&
       IMP::atom::get_chain(rd).get_id() == chain) return rd;
  }
  std::cerr << "Residue not found " << res_index << chain << std::endl;
  exit(0);
}

void read_connect_chains_file(const std::string& file_name,
                              const IMP::ParticlesTemp& atoms,
                              std::vector<IMP::atom::Atoms>& connect_atoms) {

  std::ifstream in_file(file_name.c_str());
  if(!in_file) {
    std::cerr << "Can't find rotatable bonds file " << file_name << std::endl;
    exit(1);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if(split_results.size() != 2) continue;

    IMP::atom::Atoms connecting_atoms;
    for(int i=0; i<2; i++) {
      int atom_index = atoi(split_results[i].c_str());
      connecting_atoms.push_back(find_atom(atoms, atom_index));
    }
    connect_atoms.push_back(connecting_atoms);
  }
  std::cerr << connect_atoms.size() << " chain connecting atoms "
            << " were read from file "  << file_name << std::endl;
}

void read_angle_file(const std::string& file_name,
                     const IMP::ParticlesTemp& residues,
                     const IMP::ParticlesTemp& atoms,
                     IMP::atom::Residues& flexible_residues,
                     std::vector<IMP::atom::Atoms>& dihedral_angles) {

  std::ifstream in_file(file_name.c_str());
  if(!in_file) {
    std::cerr << "Can't find rotatable bonds file " << file_name << std::endl;
    exit(1);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if(split_results.size() <= 2) {
      float res_number = atoi(split_results[0].c_str());
      std::string chain_id = " ";
      if(split_results.size() == 2) {
        chain_id = split_results[1];
      }
      flexible_residues.push_back(find_residue(residues, res_number, chain_id));
    }
    if(split_results.size() == 4) {
      IMP::atom::Atoms angle_atoms;
      for(int i=0; i<4; i++) {
        int atom_index = atoi(split_results[i].c_str());
        angle_atoms.push_back(find_atom(atoms, atom_index));
      }
      dihedral_angles.push_back(angle_atoms);
    }
  }
  std::cerr << flexible_residues.size() << " residues "
            << dihedral_angles.size() << " bonds "
            << " were read from file "  << file_name << std::endl;

}

IMP::atom::Bond create_bond(IMP::atom::Atoms& as) {
  IMP::atom::Bonded b[2];
  for(unsigned int i = 0; i < 2; ++i) {
    if(IMP::atom::Bonded::get_is_setup(as[i]))
      b[i] = IMP::atom::Bonded(as[i]);
    else
      b[i] = IMP::atom::Bonded::setup_particle(as[i]);
  }
  IMP::atom::Bond bd = IMP::atom::get_bond(b[0], b[1]);
  if(bd == IMP::atom::Bond()) {
    bd = IMP::atom::create_bond(b[0], b[1], IMP::atom::Bond::SINGLE);
    return bd;
  } else return IMP::atom::Bond();
}

void add_missing_bonds(IMP::ParticlesTemp& atoms, IMP::ParticlesTemp& bonds) {
  float thr2 = 2.0*2.0;
  std::vector<IMP::algebra::Vector3D> coordinates;
  IMP::saxs::get_coordinates(atoms, coordinates);
  int counter = 0;
  for(unsigned int i=0; i<atoms.size(); i++) {
    for(unsigned int j=i+1; j<atoms.size(); j++) {
      float dist2 = IMP::algebra::get_squared_distance(coordinates[i], coordinates[j]);
      if(dist2 < thr2) { // add bond
        IMP::atom::Atoms as;
        IMP::atom::Atom ai = IMP::atom::Atom(atoms[i]);
        IMP::atom::Atom aj = IMP::atom::Atom(atoms[j]);
        as.push_back(ai);
        as.push_back(aj);

        IMP::atom::Bonded b[2];
        for(unsigned int i = 0; i < 2; ++i) {
          if(IMP::atom::Bonded::get_is_setup(as[i]))
            b[i] = IMP::atom::Bonded(as[i]);
          else
            b[i] = IMP::atom::Bonded::setup_particle(as[i]);
        }
        IMP::atom::Bond bd = IMP::atom::get_bond(b[0], b[1]);
        if(bd == IMP::atom::Bond()) {
          bd = IMP::atom::create_bond(b[0], b[1], IMP::atom::Bond::SINGLE);
          bonds.push_back(bd);
          counter++;
        }
      }
    }
  }
  std::cerr << counter << " bonds were added" << std::endl;
}
}

using namespace IMP::kinematics;

int main(int argc, char **argv)
{
  // output arguments
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  int number_of_iterations = 100;
  int number_of_nodes = 100;
  int save_configuration_number = 10;
  int number_of_models_in_pdb = 100;
  int number_of_active_dofs = 0;
  float radii_scaling = 0.5;
  bool reset_angles = false;
  std::string configuration_file;
  std::string connect_chains_file;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "PDB file and Rotatable Angles file")
    ("version", "Written by Dina Schneidman.")
    ("number_of_iterations,i", po::value<int>(&number_of_iterations)->default_value(100),
     "number of iterations (default = 100)")
    ("number_of_nodes,n", po::value<int>(&number_of_nodes)->default_value(100),
     "number of nodes (default = 100)")
    ("number_of_path_configurations_saved,p", po::value<int>(&save_configuration_number)->default_value(10),
     "if the path between two nodes is feasible, each Nth configuration on the path will be added to the tree (default = 10?)")
    ("number_of_active_dofs,a", po::value<int>(&number_of_active_dofs)->default_value(0),
     "for many dofs use this option with 10-50 dofs (default = 0)")
    ("radii_scaling,s", po::value<float>(&radii_scaling)->default_value(0.5),
     "radii scaling parameter (0.5 < s < 1.0, default = 0.5)")
    ("reset_angles", "set initial values in rotatable angles to PI (default = false)")
    ("connect_chains_file,c",
     po::value<std::string>(&connect_chains_file),
     "connect chains into one rigid body by adding bonds between specified atoms")
    ("number_of_models_in_pdb,m", po::value<int>(&number_of_models_in_pdb)->default_value(100),
     "number of models in output PDB files (default = 100)")
    ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and rotatable angles files")
    ("target_configurations,t", po::value<std::string>(&configuration_file),
     "target_configurations")
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
    std::cerr << "radii_scaling parameter outside allowed range" << radii_scaling << std::endl;
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
  IMP_NEW(IMP::container::ListSingletonContainer, lsc, (atoms));
  IMP_NEW(IMP::container::ClosePairContainer, cpc, (lsc, 15.0));
  cpc->add_pair_filter(pair_filter);

  IMP_NEW(IMP::core::SoftSpherePairScore,  score,(1));
  IMP_NEW(IMP::container::PairsRestraint, pr, (score, cpc));

  // TODO: check why not working: should be much faster
  //IMP::Pointer<IMP::Restraint> pr=
  //   IMP::container::create_restraint(score, cpc);

  ProteinKinematics pk(mhd, flexible_residues, dihedral_angles);
  std::cerr << "ProteinKinematics done" << std::endl;
  DihedralAngleRevoluteJoints joints = pk.get_ordered_joints();
  IMP_NEW(KinematicForestScoreState, kfss, (pk.get_kinematic_forest(),
                                            pk.get_rigid_bodies(),
                                            atoms));
  model->add_score_state(kfss);

  // create dofs
  DOFs dofs;
  for(unsigned int i=0; i<joints.size(); i++) {
    std::cerr << "Angle = " << joints[i]->get_angle() << " " << 180 * joints[i]->get_angle()/IMP::algebra::PI << std::endl;
    if(reset_angles) joints[i]->set_angle(IMP::algebra::PI);
    IMP_NEW(DOF, dof, (joints[i]->get_angle(),
                       -IMP::algebra::PI,
                       IMP::algebra::PI,
                       IMP::algebra::PI/360));
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


  std::cerr << "Start RRT run" << std::endl;
  std::string filename = "node_begin.pdb";
  IMP::atom::write_pdb(mhd, filename);
  rrt->run();
  std::cerr << "Done RRT " << rrt->get_DOFValues().size() << std::endl;

  // output PDBs
  std::ofstream *out = NULL;
  int file_counter = 1;
  int model_counter = 0;

  std::vector<DOFValues> dof_values = rrt->get_DOFValues();
  for(unsigned int i = 0; i<dof_values.size(); i++) {
    ub_sampler->apply(dof_values[i]);
    kfss->do_before_evaluate();

    // open new file if needed
    if(model_counter % number_of_models_in_pdb == 0) { // open new file
      if(out !=NULL) out->close();
      std::string file_name = "nodes" + std::string(boost::lexical_cast<std::string>(file_counter)) + ".pdb";
      out = new std::ofstream(file_name.c_str());
      file_counter++;
      model_counter=0;
    }

    IMP::atom::write_pdb(mhd, *out, model_counter+1);
    model_counter++;
  }
  out->close();
  return 0;
}
