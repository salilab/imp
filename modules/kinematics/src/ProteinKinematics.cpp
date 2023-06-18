/**
 * \file ProteinKinematics
 * \brief functionality for defining a kinematic forest for proteins
 *
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 *  \authors Dina Schneidman, Barak Raveh
 *
 */

#include <IMP/kinematics/ProteinKinematics.h>
#include <IMP/kinematics/internal/graph_helpers.h>

#include <IMP/atom/dihedrals.h>
#include <IMP/exception.h>

#include <boost/graph/connected_components.hpp>
#include <boost/graph/undirected_dfs.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

ProteinKinematics::ProteinKinematics(atom::Hierarchy mhd,
                                     bool flexible_backbone,
                                     bool flexible_side_chains)
  : Object("ProteinKinematics%1%"), mhd_(mhd),
    atom_particles_(atom::get_by_type(mhd_, atom::ATOM_TYPE)),
    graph_(atom_particles_.size()) 
{
  init(atom::get_by_type(mhd, atom::RESIDUE_TYPE),
       ParticleIndexQuads(),
       std::vector<atom::Atoms>(),
       std::vector<ProteinAngleType>(),
       atom::Atoms(),
       flexible_backbone,
       flexible_side_chains);
}

ProteinKinematics::ProteinKinematics(atom::Hierarchy mhd,
                                     const atom::Residues& flexible_residues,
                                     const ParticleIndexQuads custom_dihedral_angles,
                                     atom::Atoms open_loop_bond_atoms,
                                     bool flexible_backbone,
                                     bool flexible_side_chains )
  : Object("ProteinKinematics%1%"), mhd_(mhd),
    atom_particles_(atom::get_by_type(mhd_, atom::ATOM_TYPE)),
    graph_(atom_particles_.size())
{
  init(flexible_residues,
       custom_dihedral_angles,
       std::vector<atom::Atoms>(),
       std::vector<ProteinAngleType>(),
       open_loop_bond_atoms,
       flexible_backbone,
       flexible_side_chains);
}


ProteinKinematics::ProteinKinematics(atom::Hierarchy mhd,
                    const atom::Residues& flexible_residues,
                    const std::vector<atom::Atoms>& custom_dihedral_atoms,
                    atom::Atoms open_loop_bond_atoms,
                    bool flexible_backbone,
                    bool flexible_side_chains)

  : Object("ProteinKinematics%1%"), mhd_(mhd),
    atom_particles_(atom::get_by_type(mhd_, atom::ATOM_TYPE)),
    graph_(atom_particles_.size())
{
  init(flexible_residues,
       ParticleIndexQuads(),
       custom_dihedral_atoms,
       std::vector<ProteinAngleType>(),
       open_loop_bond_atoms,
       flexible_backbone,
       flexible_side_chains);
}

ProteinKinematics::ProteinKinematics(atom::Hierarchy mhd,
                                     const atom::Residues& flexible_residues,
                                     const ParticleIndexQuads custom_dihedral_angles,
                                     const std::vector<ProteinAngleType>& custom_dihedral_angle_types,
                                     atom::Atoms open_loop_bond_atoms,
                                     bool flexible_backbone,
                                     bool flexible_side_chains )
  : Object("ProteinKinematics%1%"), mhd_(mhd),
    atom_particles_(atom::get_by_type(mhd_, atom::ATOM_TYPE)),
    graph_(atom_particles_.size())
{
//  IMP_ALWAYS_CHECK(custom_dihedral_angles.size()>custom_dihedral_angle_types(),
//       "there should be at least the same number of custom angles as custom angle types");
  init(flexible_residues,
       custom_dihedral_angles,
       std::vector<atom::Atoms>(),
       custom_dihedral_angle_types,
       open_loop_bond_atoms,
       flexible_backbone,
       flexible_side_chains);
}


void ProteinKinematics::init(const atom::Residues& flexible_residues,
                             const ParticleIndexQuads custom_dihedral_angles,
                             const std::vector<atom::Atoms>& custom_dihedral_atoms,
              const std::vector<ProteinAngleType>& custom_dihedral_angle_types,
                             atom::Atoms open_loop_bond_atoms,
                             bool flexible_backbone,
                             bool flexible_side_chains)
{
  kf_ = new kinematics::KinematicForest(mhd_.get_model());

  // 0. Are we given custom_dihedral_atoms or custom_dihedral_angles?
  // Convert custom_dihedral_angles into atom lists
  std::vector<atom::Atoms> custom_dihedral_angles_atoms;
  if (custom_dihedral_atoms.size() == 0) {
    if (custom_dihedral_angles.size() > 0) {custom_dihedral_angles_atoms = quick_hack_converter(mhd_.get_model(), custom_dihedral_angles);}
  } else { custom_dihedral_angles_atoms = custom_dihedral_atoms; }

  // 1. update graph_ with the topology from mhd_
  build_topology_graph();

  // 2. define and mark all custom rotatable dihedral angles
  mark_rotatable_angles(custom_dihedral_angles_atoms);

  // 3. define and mark all phi angles, if flexible_backbone is true
  std::vector<atom::Atoms> phi_angles, psi_angles;
  std::vector<atom::Residue> phi_residues, psi_residues;
  if (flexible_backbone) {
    for (unsigned int i = 0; i < flexible_residues.size(); i++) {
      atom::Atoms phi_atoms =
          atom::get_phi_dihedral_atoms(flexible_residues[i]);
      if (phi_atoms.size() == 4 &&
          // proline residue has a fixed phi angle (-60 degrees)
          flexible_residues[i].get_residue_type() != atom::PRO) {
        phi_angles.push_back(phi_atoms);
        phi_residues.push_back(flexible_residues[i]);
      }

      atom::Atoms psi_atoms =
          atom::get_psi_dihedral_atoms(flexible_residues[i]);
      if (psi_atoms.size() == 4) {
        psi_angles.push_back(psi_atoms);
        psi_residues.push_back(flexible_residues[i]);
      }
    }
    mark_rotatable_angles(phi_angles);
    mark_rotatable_angles(psi_angles);
  }

  open_loop(open_loop_bond_atoms);

  // 4. Mark all chi angles, if flexible_side_chains is true

  std::vector<atom::Atoms> chi1_angles, chi2_angles, chi3_angles,
      chi4_angles, chi5_angles;
  std::vector<atom::Residue> chi1_residues, chi2_residues, chi3_residues,
      chi4_residues, chi5_residues;

  if (flexible_side_chains) {
  // 4.1 Add chi angles
    for (unsigned int i = 0; i < flexible_residues.size(); i++) {
      std::vector< atom::Atoms > chi_atoms_list =
          atom::get_chi_dihedral_atoms(flexible_residues[i]);
          // Returns a list of all chi dihedrals for that residue
          for (unsigned int j = 0; j < chi_atoms_list.size(); j++) {
            // Chi values come in ascending order.
            if (j==0) {
              //std::cout << chi_atoms_list[j] << std::endl;
              chi1_angles.push_back(chi_atoms_list[j]);
              chi1_residues.push_back(flexible_residues[i]); 
            } else if (j==1) {
              chi2_angles.push_back(chi_atoms_list[j]);
              chi2_residues.push_back(flexible_residues[i]); 
            } else if (j==2) {
              chi3_angles.push_back(chi_atoms_list[j]);
              chi3_residues.push_back(flexible_residues[i]); 
            } else if (j==3) {
              chi4_angles.push_back(chi_atoms_list[j]);
              chi4_residues.push_back(flexible_residues[i]); 
            } else {
              chi5_angles.push_back(chi_atoms_list[j]);
              chi5_residues.push_back(flexible_residues[i]);
            }
          }
    }
    mark_rotatable_angles(chi1_angles);
    mark_rotatable_angles(chi2_angles);
    mark_rotatable_angles(chi3_angles);
    mark_rotatable_angles(chi4_angles);
    mark_rotatable_angles(chi5_angles);
  }

  // 3. build rbs and sort them by dfs
  build_rigid_bodies();
  order_rigid_bodies(custom_dihedral_angles_atoms, phi_angles, psi_angles, 
    chi1_angles, chi2_angles, chi3_angles, chi4_angles, chi5_angles, open_loop_bond_atoms);


  // 4. add joints to kf

  //first add the custom_dihedral_angles_atoms
  for (unsigned int i = 0; i < custom_dihedral_angles_atoms.size(); i++) {

    ProteinAngleType angle_type = OTHER;
    // Get the angle type (default to OTHER if not specified)
    if (i < custom_dihedral_angle_types.size()) { angle_type = custom_dihedral_angle_types[i];
    }

    // Get the residue that we will assign this to
    atom::Residue res = atom::Residue(custom_dihedral_angles_atoms[i][1].get_parent());
    add_dihedral_joint(res, angle_type, custom_dihedral_angles_atoms[i]);

  }


  //add_dihedral_joints(custom_dihedral_angles_atoms, custom_dihedral_angle_types);
  if (flexible_backbone) {
    add_dihedral_joints(phi_residues, PHI, phi_angles);
    add_dihedral_joints(psi_residues, PSI, psi_angles);
  }
  if (flexible_side_chains) {
    add_dihedral_joints(chi1_residues, CHI1, chi1_angles);
    add_dihedral_joints(chi2_residues, CHI2, chi2_angles);
    add_dihedral_joints(chi3_residues, CHI3, chi3_angles);
    add_dihedral_joints(chi4_residues, CHI4, chi4_angles);
    add_dihedral_joints(chi5_residues, CHI5, chi5_angles);
  }

  std::cerr << joints_.size() << " joints were constructed " << std::endl;

  // identify loop joints
  if(open_loop_bond_atoms.size() > 0) {
    // trace loop
    // find the two rigid bodies that open the loop
    Particle* p1 = open_loop_bond_atoms[0].get_particle();
    Particle* p2 = open_loop_bond_atoms[1].get_particle();
    core::RigidBody rb1 = core::RigidMember(p1).get_rigid_body();
    core::RigidBody rb2 = core::RigidMember(p2).get_rigid_body();
    int rb_index1 = rb_particle_index_to_node_map_[rb1->get_index()];
    int rb_index2 = rb_particle_index_to_node_map_[rb2->get_index()];
    int end_vertex = rb_index1;
    // start from the larger one
    if(rbs_[rb_index1].get_number_of_members() <
       rbs_[rb_index2].get_number_of_members()) {
      end_vertex = rb_index2;
    }
    int curr_vertex = (end_vertex == rb_index1) ? rb_index2 : rb_index1;
    std::cerr << "curr_vertex " << curr_vertex << " end_vertex " << end_vertex << std::endl;
    while(curr_vertex != end_vertex && loop_joints_.size() < joints_.size()) {
      std::cerr << curr_vertex << " parent " << parents_[curr_vertex] << std::endl;
      Pointer<DihedralAngleRevoluteJoint> joint = rigid_bodies_2_joint_map_[curr_vertex][parents_[curr_vertex]];
      loop_joints_.push_back(joint);
      curr_vertex = parents_[curr_vertex];
    }
  }
}

void ProteinKinematics::add_edges_to_rb_graph(
                              const std::vector<atom::Atoms>& dihedral_angles) {
  for (unsigned int i = 0; i < dihedral_angles.size(); i++) {
    Particle* p1 = dihedral_angles[i][1].get_particle();
    Particle* p2 = dihedral_angles[i][2].get_particle();
    if (core::RigidMember::get_is_setup(p1) &&
        core::RigidMember::get_is_setup(p2)) {
      core::RigidBody rb1 = core::RigidMember(p1).get_rigid_body();
      core::RigidBody rb2 = core::RigidMember(p2).get_rigid_body();
      int rb_index1 = rb_particle_index_to_node_map_[rb1->get_index()];
      int rb_index2 = rb_particle_index_to_node_map_[rb2->get_index()];
      boost::add_edge(rb_index1, rb_index2, rb_graph_);
      std::cerr << "connecting " << rb_index1 << " " << rb_index2 << std::endl;
    }
  }
}

void ProteinKinematics::build_topology_graph() {
  // map graph nodes (=atoms) to ParticleIndex
  for (unsigned int i = 0; i < atom_particles_.size(); i++) {
    //for all atom_particles_, get the ParticleIndex and put that in 
    // particle_index_to_node_map_ (pindex :: i)
    // Append pindex to node_to_particle_index_map_
    ParticleIndex pindex = atom_particles_[i]->get_index();
    particle_index_to_node_map_[pindex] = i;
    node_to_particle_index_map_.push_back(pindex);
  }

  // add edges to graph_
  atom::Bonds bonds = atom::get_internal_bonds(mhd_);
  //std::cout << "# Bonds = " << bonds.size() << std::endl;
  for (unsigned int i = 0; i < bonds.size(); i++) {
    atom::Bonded p1 = atom::Bond(bonds[i]).get_bonded(0);
    atom::Bonded p2 = atom::Bond(bonds[i]).get_bonded(1);
    int atom_index1 = particle_index_to_node_map_[p1->get_index()];
    int atom_index2 = particle_index_to_node_map_[p2->get_index()];
    boost::add_edge(atom_index1, atom_index2, graph_);
    //std::cout << "Bond added between atom " << atom::Atom(p1) << " " << atom::Atom(p2) << std::endl;
  }

  // TODO: add IMP_CHECK on this code
  std::vector<int> component(boost::num_vertices(graph_));
  // unsigned int num = boost::connected_components(graph_, &component[0]);
  //std::cout << "Num Connected Components = " << num << std::endl;
}

void ProteinKinematics::mark_rotatable_angles(
                             const std::vector<atom::Atoms>& dihedral_angles) {
  for (unsigned int i = 0; i < dihedral_angles.size(); i++) {
    mark_rotatable_angle(dihedral_angles[i]);
  }
}

void ProteinKinematics::mark_rotatable_angle(
                             const std::vector<atom::Atom>& dihedral_angle) {
  // get the ParticleIndex and map it to graph node
  ParticleIndex p1 = dihedral_angle[1].get_particle_index();
  ParticleIndex p2 = dihedral_angle[2].get_particle_index();
  int atom_index1 = 0;
  int atom_index2 = 0;

  if (particle_index_to_node_map_.find(p1) !=
      particle_index_to_node_map_.end()) {
    atom_index1 = particle_index_to_node_map_[p1];
  } else {
    IMP_THROW("cannot find node index for angle", ValueException);
  }
  if (particle_index_to_node_map_.find(p2) !=
      particle_index_to_node_map_.end()) {
    atom_index2 = particle_index_to_node_map_[p2];
  } else {
    IMP_THROW("cannot find node index for angle", ValueException);
  }
  std::vector<int> component(boost::num_vertices(graph_));
  // unsigned int num = boost::connected_components(graph_, &component[0]);
  //std::cerr << "CC NUM before removal of rotatable bonds = " << component[0]
  //          << std::endl;
  //std::cerr << "REMOVE EDGE = " << atom_index1 << atom_index2 << std::endl;
  boost::remove_edge(atom_index1, atom_index2, graph_);
  std::vector<int> component2(boost::num_vertices(graph_));
  unsigned int num1 = boost::connected_components(graph_, &component2[0]);
  IMP_UNUSED(num1);
  //std::cerr << "CC NUM after removal of rotatable bonds = " << component2[0]
  //          << std::endl;
}


void ProteinKinematics::open_loop(atom::Atoms open_loop_bond_atoms) {
  if(open_loop_bond_atoms.size()==2) {
    // get the ParticleIndex and map it to graph node
    ParticleIndex p1 = open_loop_bond_atoms[0].get_particle_index();
    ParticleIndex p2 = open_loop_bond_atoms[1].get_particle_index();
    int atom_index1 = 0;
    int atom_index2 = 0;
    if(particle_index_to_node_map_.find(p1) !=
       particle_index_to_node_map_.end()) {
      atom_index1 = particle_index_to_node_map_[p1];
    } else {
      IMP_THROW("cannot find node index for angle", ValueException);
    }
    if(particle_index_to_node_map_.find(p2) !=
       particle_index_to_node_map_.end()) {
      atom_index2 = particle_index_to_node_map_[p2];
    } else {
      IMP_THROW("cannot find node index for angle", ValueException);
    }

    boost::remove_edge(atom_index1, atom_index2, graph_);
  }
}

void ProteinKinematics::build_rigid_bodies() {
  // compute connected components that represent rigid parts
  std::vector<int> component(boost::num_vertices(graph_));
  unsigned int num = boost::connected_components(graph_, &component[0]);

  // store the atoms of each rigid body using node index
  std::vector<std::vector<int> > rigid_bodies_atoms(num);
  for (unsigned int i = 0; i < component.size(); i++) {
    rigid_bodies_atoms[component[i]].push_back(i);
  }
  // build the rigid bodies
  Model* m = mhd_->get_model();
  largest_rb_ = 0;
  unsigned int largest_rb_size = 0;
  for (unsigned int i = 0; i < rigid_bodies_atoms.size(); i++) { // i = connected-componenet number
    Particle* rbp = new Particle(m);
    std::string name = "rb_name";  // TODO: add rb id
    rbp->set_name(name);
    // rb atoms, get Particles from node indexes
    ParticlesTemp all;
    for (unsigned int j = 0; j < rigid_bodies_atoms[i].size(); j++) {
      all.push_back(m->get_particle(
          node_to_particle_index_map_[rigid_bodies_atoms[i][j]]));
    }
    core::RigidBody rbd =
        core::RigidBody::setup_particle(rbp, core::XYZs(all));
    rbd.set_coordinates_are_optimized(true);
    rbs_.push_back(rbd);
    rb_particle_index_to_node_map_[rbp->get_index()] = i;
    // update largest rb
    if(rigid_bodies_atoms[i].size() > largest_rb_size) {
      largest_rb_size = rigid_bodies_atoms[i].size();
      largest_rb_ = i;
    }
    std::cerr << "Rb " << i << " created with " << rigid_bodies_atoms[i].size()
              << " atoms" << std::endl;
  }
  std::cerr << rbs_.size() << " rigid bodies were created " << std::endl;
}

void ProteinKinematics::order_rigid_bodies(
                         const std::vector<atom::Atoms>& custom_dihedral_angles,
                         const std::vector<atom::Atoms>& phi_angles,
                         const std::vector<atom::Atoms>& psi_angles,
                         const std::vector<atom::Atoms>& chi1_angles,
                         const std::vector<atom::Atoms>& chi2_angles,
                         const std::vector<atom::Atoms>& chi3_angles,
                         const std::vector<atom::Atoms>& chi4_angles,
                         const std::vector<atom::Atoms>& chi5_angles,
                         atom::Atoms open_loop_bond_atoms) {

  // build rigid bodies topology graph
  for(unsigned int i=0; i<rbs_.size(); i++)
    boost::add_vertex(rb_graph_); // add nodes

  // add edges
  add_edges_to_rb_graph(custom_dihedral_angles);
  add_edges_to_rb_graph(phi_angles);
  add_edges_to_rb_graph(psi_angles);

  add_edges_to_rb_graph(chi1_angles);
  add_edges_to_rb_graph(chi2_angles);
  add_edges_to_rb_graph(chi3_angles);
  add_edges_to_rb_graph(chi4_angles);
  add_edges_to_rb_graph(chi5_angles);

  rb_order_.resize(rbs_.size());
  parents_.resize(rbs_.size());

  // run DFS
  internal::MyDFSVisitor vis(rb_order_, parents_);
  int starting_vertex = largest_rb_;
  int rb_index1(0), rb_index2(0);
  if(open_loop_bond_atoms.size() > 0) {
    // find the two rigid bodies that open the loop
    Particle* p1 = open_loop_bond_atoms[0].get_particle();
    Particle* p2 = open_loop_bond_atoms[1].get_particle();
    core::RigidBody rb1 = core::RigidMember(p1).get_rigid_body();
    core::RigidBody rb2 = core::RigidMember(p2).get_rigid_body();
    rb_index1 = rb_particle_index_to_node_map_[rb1->get_index()];
    rb_index2 = rb_particle_index_to_node_map_[rb2->get_index()];
    starting_vertex = rb_index1;
    // start from the larger one
    if(rbs_[rb_index1].get_number_of_members() <
       rbs_[rb_index2].get_number_of_members()) {
      starting_vertex = rb_index2;
    }
  }

  boost::depth_first_search(rb_graph_, visitor(vis).root_vertex(starting_vertex));

  for(unsigned int i=0; i<rb_order_.size(); i++) {
    std::cerr << "rb order " << i << " --> " << rb_order_[i]
              << " parent " << parents_[rb_order_[i]] << std::endl;
  }
}
/*
void ProteinKinematics::add_dihedral_joints
( const std::vector<atom::Atoms>& dihedral_angles,
  const std::vector<ProteinAngleType>& dihedral_angle_types)
{
  for (unsigned int i = 0; i < dihedral_angles.size(); i++) {
    ProteinAngleType type= OTHER;
    if(i<dihedral_angle_types.size()){
      type=dihedral_angle_types[i];
    }
    add_dihedral_joint(atom::get_residue(dihedral_angles[i][1]),
           type,
           dihedral_angles[i]);
  }
}
*/
void ProteinKinematics::add_dihedral_joints(
    const std::vector<atom::Residue>& residues,
    ProteinAngleType angle_type,
    const std::vector<atom::Atoms>& dihedral_angles) {

  for (unsigned int i = 0; i < dihedral_angles.size(); i++){
    add_dihedral_joint(residues[i], angle_type, dihedral_angles[i]);
  }
}

void ProteinKinematics::add_dihedral_joint(const atom::Residue r,
                                           ProteinAngleType angle_type,
                                           const atom::Atoms& atoms) {
  Particle* p1 = atoms[1].get_particle();
  Particle* p2 = atoms[2].get_particle();

  // TODO: check to DEBUG only check?
  if (core::RigidMember::get_is_setup(p1) &&
      core::RigidMember::get_is_setup(p2)) {
    core::RigidBody rb1 = core::RigidMember(p1).get_rigid_body();
    core::RigidBody rb2 = core::RigidMember(p2).get_rigid_body();
    int rb_index1 = rb_particle_index_to_node_map_[rb1->get_index()];
    int rb_index2 = rb_particle_index_to_node_map_[rb2->get_index()];
    Pointer<DihedralAngleRevoluteJoint> joint;
    if(rb_order_[rb_index1] < rb_order_[rb_index2]) {
      joint = new DihedralAngleRevoluteJoint(rb1, rb2,
                                             core::XYZ(atoms[0].get_particle()),
                                             core::XYZ(atoms[1].get_particle()),
                                             core::XYZ(atoms[2].get_particle()),
                                             core::XYZ(atoms[3].get_particle()));
    } else {
      joint = new DihedralAngleRevoluteJoint(rb2, rb1,
                                             core::XYZ(atoms[3].get_particle()),
                                             core::XYZ(atoms[2].get_particle()),
                                             core::XYZ(atoms[1].get_particle()),
                                             core::XYZ(atoms[0].get_particle()));
    }
    joints_.push_back(joint);
    kf_->add_edge(joint);
    joint_map_.add_joint(r, angle_type, joint);
    rigid_bodies_2_joint_map_[rb_index1][rb_index2]=joint;
    rigid_bodies_2_joint_map_[rb_index2][rb_index1]=joint;
  } else {
    IMP_THROW("cannot find rigid bodies for dihedral angle",
              ValueException);
  }
}

Joint* ProteinKinematics::AngleToJointMap::get_joint(const atom::Residue r,
                                            ProteinAngleType angle_type) const {
  ParticleIndex pi = r.get_particle_index();
  IMP_INTERNAL_CHECK(residue_to_joints_.find(pi) != residue_to_joints_.end(),
                     "Can't find joints for residue\n");
  const ResidueJoints& res_joints = residue_to_joints_.find(pi)->second;
  IMP_INTERNAL_CHECK(res_joints.size() > angle_type,
                     "Can't find a joint for residue");
  return res_joints[angle_type];
}

void ProteinKinematics::AngleToJointMap::add_joint(const atom::Residue r,
                                                   ProteinAngleType angle_type,
                                                   Joint* joint) {
  ParticleIndex pi = r.get_particle_index();
  if (residue_to_joints_.find(pi) == residue_to_joints_.end()) {
    ResidueJoints residue_joints(angle_type + 1, nullptr);
    residue_joints[angle_type] = joint;
    residue_to_joints_[pi] = residue_joints;
  } else {
    ResidueJoints& residue_joints = residue_to_joints_[pi];
    if (residue_joints.size() <= angle_type) {
      residue_joints.resize(angle_type + 1, nullptr);
    }
    residue_joints[angle_type] = joint;
  }
}

std::vector<atom::Atoms> ProteinKinematics::quick_hack_converter
(Model* m, const ParticleIndexQuads piqs)
{
   std::vector<atom::Atoms> ret_value;
   for(unsigned int i=0; i<piqs.size(); i++){
      atom::Atoms cur_atoms_quad;
      for(unsigned int j=0; j<4; j++) {
        IMP_INTERNAL_CHECK(IMP::atom::Atom::get_is_setup(m, piqs[i][j]), 
			   "All particle indexes in quads must be decorated with Atom");
        cur_atoms_quad.push_back(IMP::atom::Atom(m, piqs[i][j]));
      } // for j
      ret_value.push_back(cur_atoms_quad);
   } // for i
   return ret_value;
} // end of function

IMPKINEMATICS_END_NAMESPACE
