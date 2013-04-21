/**
 * \file ProteinKinematics
 * \brief functionality for defining a kinematic forest for proteins
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *  \authors Dina Schneidman, Barak Raveh
 *
 */

#ifndef IMPKINEMATICS_PROTEIN_KINEMATICS_H
#define IMPKINEMATICS_PROTEIN_KINEMATICS_H

#include "kinematics_config.h"

#include <IMP/kinematics/revolute_joints.h>
#include <IMP/kinematics/KinematicForest.h>

#include <IMP/core/rigid_bodies.h>
#include <IMP/atom/Atom.h>


#include <IMP/base/map.h>
#include <IMP/base/Vector.h>
#include <vector>
#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

typedef boost::adjacency_list <boost::vecS,
                               boost::vecS,
                               boost::undirectedS,
                               boost::no_property,
    boost::property<boost::edge_color_t, boost::default_color_type> > Graph;

class IMPKINEMATICSEXPORT ProteinKinematics {
 public:
  /* Constructors */

  // all phi/psi rotatable
  ProteinKinematics(IMP::atom::Hierarchy mhd,
                    bool flexible_backbone = true,
                    bool flexible_side_chains = false);

  // only torsions from dihedral_angles list are rotatable
  ProteinKinematics(IMP::atom::Hierarchy mhd,
                    const IMP::atom::Residues& flexible_residues,
                    const std::vector<IMP::atom::Atoms>& dihedral_angles,
                    bool flexible_backbone = true,
                    bool flexible_side_chains = false);

 private:

  //! the actual construction is done here,
  //! see constructors for documentation
  void init( const IMP::atom::Residues& flexible_residues,
             const std::vector<IMP::atom::Atoms>& dihedral_angles,
             bool flexible_backbone,
             bool flexible_side_chains) ;

 public:


  /* Access methods */

  double get_phi(const IMP::atom::Residue r) const {
    return get_phi_joint(r)->get_angle();
  }

  double get_psi(const IMP::atom::Residue r) const {
    return get_psi_joint(r)->get_angle();
  }

  DihedralAngleRevoluteJoints get_joints() { return joints_; }

  KinematicForest* get_kinematic_forest() { return kf_; }
  // TODO: not sure if we have to return Pointer or just raw pointer

  IMP::core::RigidBodies get_rigid_bodies() { return rbs_; }


  // TODO: add chi

  /* Modifier methods */

  void set_phi(const IMP::atom::Residue r, double angle) {
    get_phi_joint(r)->set_angle(angle);
    kf_->update_all_external_coordinates();
  }

  void set_psi(const IMP::atom::Residue r, double angle) {
    get_psi_joint(r)->set_angle(angle);
    kf_->update_all_external_coordinates();
  }

  // TODO: add chi


private:

  enum ProteinAngleType { PHI, PSI, CHI1, CHI2, CHI3, CHI4, OTHER, TOTAL };

  void build_topology_graph();

  void mark_rotatable_angles(
                        const std::vector<IMP::atom::Atoms>& dihedral_angles);

  void build_rigid_bodies();

  void add_dihedral_joints(
                          const std::vector<IMP::atom::Atoms>& dihedral_angles);

  void add_dihedral_joints(const std::vector<IMP::atom::Residue>& residues,
                           ProteinAngleType angle_type,
                          const std::vector<IMP::atom::Atoms>& dihedral_angles);

  void add_dihedral_joint(const IMP::atom::Residue r,
                          ProteinAngleType angle_type,
                          const IMP::atom::Atoms& atoms);

  /* Joint access methods */
  DihedralAngleRevoluteJoint* get_phi_joint(const IMP::atom::Residue r) const {
    return (DihedralAngleRevoluteJoint*)joint_map_.get_joint(r, PHI);
  }

  DihedralAngleRevoluteJoint* get_psi_joint(const IMP::atom::Residue r) const {
    return (DihedralAngleRevoluteJoint*)joint_map_.get_joint(r, PSI);
  }

  //DihedralAngleRevoluteJoints get_joints(const IMP::atom::Residue r) const;

  // map between residue phi/psi and joints
  class AngleToJointMap {
  public:
    // Joint access
    Joint* get_joint(const IMP::atom::Residue r,
                     ProteinAngleType angle_type) const;

    // store Joint
    void add_joint(const IMP::atom::Residue r,
                   ProteinAngleType angle_type,
                   Joint* joint);

  private:
    /* mapping to phi/psi/chi for a specific residue.
       the joints are stored using ProteinAngleType as an index */
    typedef std::vector<Joint*> ResidueJoints;
    /* mapping between residue and its joints */
    IMP::base::map<IMP::ParticleIndex,
                            ResidueJoints> residue_to_joints_;
  };

 private:
  // protein hierarchy
  IMP::atom::Hierarchy mhd_;

  // atom particles
  IMP::ParticlesTemp atom_particles_;

  // topology graph: nodes = atoms, edges = bonds
  Graph graph_;

  // mapping between atom ParticleIndex and node number in the graph
  IMP::base::map<IMP::ParticleIndex, int> particle_index_to_node_map_;
  IMP::base::Vector<IMP::ParticleIndex> node_to_particle_index_map_;

  // rigid bodies
  IMP::core::RigidBodies rbs_;

  // joints
  DihedralAngleRevoluteJoints joints_;

  IMP::base::Pointer<IMP::kinematics::KinematicForest> kf_;

  // map between residue phi/psi/chis and joints
  AngleToJointMap joint_map_;
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_PROTEIN_KINEMATICS_H */
