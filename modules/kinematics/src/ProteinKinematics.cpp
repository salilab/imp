/**
 * \file ProteinKinematics
 * \brief functionality for defining a kinematic forest for proteins
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *  \authors Dina Schneidman, Barak Raveh
 *
 */

#include <IMP/kinematics/ProteinKinematics.h>

#include <IMP/atom/dihedrals.h>
#include <IMP/exception.h>

#include <boost/graph/connected_components.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

ProteinKinematics::ProteinKinematics(IMP::atom::Hierarchy mhd,
                                     bool flexible_backbone,
                                     bool flexible_side_chains) :
  ProteinKinematics(mhd,
                    IMP::atom::get_by_type(mhd, IMP::atom::RESIDUE_TYPE),
                    std::vector<IMP::atom::Atoms>(),
                    flexible_backbone,
                    flexible_side_chains)
{}

ProteinKinematics::ProteinKinematics(IMP::atom::Hierarchy mhd,
                          const IMP::atom::Residues& flexible_residues,
                          const std::vector<IMP::atom::Atoms>& dihedral_angles,
                          bool flexible_backbone,
                          bool flexible_side_chains) :
  mhd_(mhd),
  atom_particles_(IMP::atom::get_by_type(mhd_, IMP::atom::ATOM_TYPE)),
  graph_(atom_particles_.size())
{
  kf_ = new IMP::kinematics::KinematicForest(mhd_.get_model());

  // 1.
  build_topology_graph();

  // 2. define and mark rotatable dihedral angles
  mark_rotatable_angles(dihedral_angles);

  // get the angles
  std::vector<IMP::atom::Atoms> phi_angles, psi_angles;
  std::vector<IMP::atom::Residue> phi_residues, psi_residues;
  if(flexible_backbone) {
    for(unsigned int i=0; i<flexible_residues.size(); i++) {
      IMP::atom::Atoms phi_atoms =
        IMP::atom::get_phi_dihedral_atoms(flexible_residues[i]);
      if(phi_atoms.size() == 4) {
        phi_angles.push_back(phi_atoms);
        phi_residues.push_back(flexible_residues[i]);
      }

      IMP::atom::Atoms psi_atoms =
        IMP::atom::get_psi_dihedral_atoms(flexible_residues[i]);
      if(psi_atoms.size() == 4) {
        psi_angles.push_back(psi_atoms);
        psi_residues.push_back(flexible_residues[i]);
      }
    }
    mark_rotatable_angles(phi_angles);
    mark_rotatable_angles(psi_angles);
  }

  std::vector<IMP::atom::Atoms> chi1_angles, chi2_angles,
    chi3_angles, chi4_angles;
  std::vector<IMP::atom::Residue> chi1_residues, chi2_residues,
    chi3_residues, chi4_residues;
  if(flexible_side_chains) {
    // TODO
  }

  // 3.
  build_rigid_bodies();

  // 4. add joints to kf
  add_dihedral_joints(dihedral_angles);
  if(flexible_backbone) {
    add_dihedral_joints(phi_residues, PHI, phi_angles);
    add_dihedral_joints(psi_residues, PSI, psi_angles);
  }
  if(flexible_side_chains) {
    add_dihedral_joints(chi1_residues, CHI1, chi1_angles);
    add_dihedral_joints(chi2_residues, CHI2, chi2_angles);
    add_dihedral_joints(chi3_residues, CHI3, chi3_angles);
    add_dihedral_joints(chi4_residues, CHI4, chi4_angles);
  }

  std::cerr << joints_.size() << " joints were constructed " << std::endl;
}

void ProteinKinematics::build_topology_graph() {

  // map graph nodes (=atoms) to ParticleIndex
  for(unsigned int i=0; i<atom_particles_.size(); i++) {
    IMP::ParticleIndex pindex = atom_particles_[i]->get_index();
    particle_index_to_node_map_[pindex] = i;
    node_to_particle_index_map_.push_back(pindex);
  }

  // add edges to graph
  IMP::atom::Bonds bonds = IMP::atom::get_internal_bonds(mhd_);
  for(unsigned int i=0; i<bonds.size(); i++) {
    IMP::atom::Bonded p1 = IMP::atom::Bond(bonds[i]).get_bonded(0);
    IMP::atom::Bonded p2 = IMP::atom::Bond(bonds[i]).get_bonded(1);
    int atom_index1 = particle_index_to_node_map_[p1->get_index()];
    int atom_index2 = particle_index_to_node_map_[p2->get_index()];
    boost::add_edge(atom_index1, atom_index2, graph_);
  }

  // TODO: add IMP_CHECK on this code
  std::vector<int> component(boost::num_vertices(graph_));
  unsigned int num = boost::connected_components(graph_, &component[0]);
  std::cerr << "CC NUM before removal of rotatable bonds = "
            << num << std::endl;
}

void ProteinKinematics::mark_rotatable_angles(
                         const std::vector<IMP::atom::Atoms>& dihedral_angles) {
  for(unsigned int i=0; i<dihedral_angles.size(); i++) {

    // get the ParticleIndex and map it to graph node
    IMP::ParticleIndex p1 = dihedral_angles[i][1].get_particle_index();
    IMP::ParticleIndex p2 = dihedral_angles[i][2].get_particle_index();
    int atom_index1 = 0;
    int atom_index2 = 0;
    if(particle_index_to_node_map_.find(p1) !=
       particle_index_to_node_map_.end()) {
      atom_index1 = particle_index_to_node_map_[p1];
    } else {
      IMP_THROW("cannot find node index for angle", IMP::ValueException);
    }
    if(particle_index_to_node_map_.find(p2) !=
       particle_index_to_node_map_.end()) {
      atom_index2 = particle_index_to_node_map_[p2];
    } else {
      IMP_THROW("cannot find node index for angle", IMP::ValueException);
    }

    boost::remove_edge(atom_index1, atom_index2, graph_);
  }
}

void ProteinKinematics::build_rigid_bodies() {
  // compute connected components that represent rigid parts
  std::vector<int> component(boost::num_vertices(graph_));
  unsigned int num = boost::connected_components(graph_, &component[0]);
  std::cerr << "CC NUM = " << num << std::endl;

  // store the atoms of each rigid body using node index
  std::vector<std::vector<int> > rigid_bodies_atoms(num);
  for(unsigned int i=0; i<component.size(); i++) {
    rigid_bodies_atoms[component[i]].push_back(i);
  }

  // build the rigid bodies
  IMP::Model* m = mhd_->get_model();
  for(unsigned int i=0; i<rigid_bodies_atoms.size(); i++) {
    IMP::Particle *rbp= new IMP::Particle(m);
    std::string name = "rb_name"; // TODO: add rb id
    rbp->set_name(name);
    // rb atoms, get Particles from node indexes
    IMP::ParticlesTemp all;
    for(unsigned int j=0; j<rigid_bodies_atoms[i].size(); j++) {
      all.push_back(
        m->get_particle(node_to_particle_index_map_[rigid_bodies_atoms[i][j]]));
    }
    IMP::core::RigidBody rbd
      = IMP::core::RigidBody::setup_particle(rbp, core::XYZs(all));
    rbd.set_coordinates_are_optimized(true);
    rbs_.push_back(rbd);
  }
  std::cerr << rbs_.size() << " rigid bodies were created " << std::endl;
}

void ProteinKinematics::add_dihedral_joints(
                      const std::vector<IMP::atom::Atoms>& dihedral_angles) {
  for(unsigned int i=0; i<dihedral_angles.size(); i++) {
    add_dihedral_joint(IMP::atom::get_residue(dihedral_angles[i][1]),
                       OTHER,
                       dihedral_angles[i]);
  }
}

void ProteinKinematics::add_dihedral_joints(
                       const std::vector<IMP::atom::Residue>& residues,
                       ProteinAngleType angle_type,
                       const std::vector<IMP::atom::Atoms>& dihedral_angles) {

  for(unsigned int i=0; i<dihedral_angles.size(); i++)
    add_dihedral_joint(residues[i], angle_type, dihedral_angles[i]);
}

void ProteinKinematics::add_dihedral_joint(const IMP::atom::Residue r,
                                           ProteinAngleType angle_type,
                                           const IMP::atom::Atoms& atoms) {

  IMP::Particle* p1 = atoms[1].get_particle();
  IMP::Particle* p2 = atoms[2].get_particle();

  // TODO: check to DEBUG only check?
  if(IMP::core::RigidMember::particle_is_instance(p1) &&
     IMP::core::RigidMember::particle_is_instance(p2)) {
    IMP::core::RigidBody rb1 = IMP::core::RigidMember(p1).get_rigid_body();
    IMP::core::RigidBody rb2 = IMP::core::RigidMember(p2).get_rigid_body();

    IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, joint,
            (rb1, rb2,
             IMP::core::XYZ(atoms[0].get_particle()),
             IMP::core::XYZ(atoms[1].get_particle()),
             IMP::core::XYZ(atoms[2].get_particle()),
             IMP::core::XYZ(atoms[3].get_particle())));

    joints_.push_back(joint);
    kf_->add_edge(joint);
    joint_map_.add_joint(r, angle_type, joint);
  } else {
    IMP_THROW("cannot find rigid bodies for dihedral angle",
              IMP::ValueException);
  }
}


Joint* ProteinKinematics::AngleToJointMap::get_joint(const IMP::atom::Residue r,
                                             ProteinAngleType angle_type) const
{
  IMP::ParticleIndex pi = r.get_particle_index();
  IMP_INTERNAL_CHECK(residue_to_joints_.find(pi) != residue_to_joints_.end(),
                     "Can't find joints for residue\n");
  const ResidueJoints& res_joints = residue_to_joints_.find(pi)->second;
  IMP_INTERNAL_CHECK(res_joints.size() > angle_type,
                     "Can't find a joint for residue");
  return res_joints[angle_type];
}

void ProteinKinematics::AngleToJointMap::add_joint(const IMP::atom::Residue r,
                                                   ProteinAngleType angle_type,
                                                   Joint* joint) {
  IMP::ParticleIndex pi = r.get_particle_index();
  if(residue_to_joints_.find(pi) == residue_to_joints_.end()) {
    ResidueJoints residue_joints(angle_type+1, nullptr);
    residue_joints[angle_type] = joint;
    residue_to_joints_[pi] = residue_joints;
  } else {
    ResidueJoints& residue_joints = residue_to_joints_[pi];
    if(residue_joints.size() <= angle_type) {
      residue_joints.resize(angle_type+1, nullptr);
    }
    residue_joints[angle_type] = joint;
  }
}


IMPKINEMATICS_END_NAMESPACE
