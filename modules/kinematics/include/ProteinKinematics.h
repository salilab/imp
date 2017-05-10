/**
 * \file IMP/kinematics/ProteinKinematics.h
 * \brief functionality for defining a kinematic forest for proteins
 *
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 *  \authors Dina Schneidman, Barak Raveh
 *
 */

#ifndef IMPKINEMATICS_PROTEINKINEMATICS_H
#define IMPKINEMATICS_PROTEINKINEMATICS_H

#include "kinematics_config.h"

#include <IMP/kinematics/revolute_joints.h>
#include <IMP/kinematics/KinematicForest.h>

#include <IMP/core/rigid_bodies.h>
#include <IMP/atom/Atom.h>

#include <boost/unordered_map.hpp>
#include <IMP/Vector.h>
#include <vector>
#include <iostream>

#include <boost/graph/adjacency_list.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

//! Kinematic structure over a protein, with backbone and side chain dihedrals
//!
//! Note: Particles that are handled by this class should never be decorated
//! with IMP::core::RigidBody externally, as this could lead to unexpected behavior
//! (e.g., wrong coordinates) and it cannot be detected by IMP at the current time.
class IMPKINEMATICSEXPORT ProteinKinematics : public IMP::Object {
 public:
  enum ProteinAngleType {
    PHI,
    PSI,
    CHI1,
    CHI2,
    CHI3,
    CHI4,
    OTHER,
    OTHER2,
    OTHER3,
    OTHER4,
    OTHER5,
    TOTAL
  };

  
 public:

  //! Constructor with all phi/psi rotatable
  /**
     @param mhd hierarchy of a protein, obtained by e.g. reading a Pdb using IMP::Atom::read_pdb()
     @param flexible_backbone whether all phi/psi angles are flexible
     @param flexible_side_chains whether all chi angles are flexible      
               (currently not implemented)
   */
  ProteinKinematics(atom::Hierarchy mhd, bool flexible_backbone = true,
                    bool flexible_side_chains = false);

  // Constructor; torsions from custom_dihedral_angles list are rotatable
  /**
     @param mhd hierarchy of a protein, obtained by e.g. reading a Pdb using IMP::Atom::read_pdb()
     @param flexible_residues all residues for which non-custom backbone of side-chain dihedrals may
                              be flexible (side-chains are not implemented as of May 2017)
     @param custom_dihedral_angles lists of four atoms that define a custom dihedral angle
     @param flexible_backbone whether all phi/psi angles in flexible_residues are flexible
     @param flexible_side_chains whether all chi angles in flexible_residues are flexible      
               (currently not implemented)
   */
  ProteinKinematics(atom::Hierarchy mhd,
                    const atom::Residues& flexible_residues,
                    const ParticleIndexQuads& custom_dihedral_angles,
                    atom::Atoms open_loop_bond_atoms = atom::Atoms(),
                    bool flexible_backbone = true,
                    bool flexible_side_chains = false);

  // Constructor; torsions from custom_dihedral_angles list are rotatable
  /**
     @param mhd hierarchy of a protein, obtained by e.g. reading a Pdb using IMP::Atom::read_pdb()
     @param flexible_residues all residues for which non-custom backbone of side-chain dihedrals may
                              be flexible (side-chains are not implemented as of May 2017)
     @param custom_dihedral_angles lists of four atoms that define a custom dihedral angle
     @param custom_dihedral_angle_types the types of all custom dihedral angles (a list of the same size)
     @param open_loop_bond_atoms TODO: document
     @param flexible_backbone whether all phi/psi angles in flexible_residues are flexible
     @param flexible_side_chains whether all chi angles in flexible_residues are flexible      
               (currently not implemented)
   */
  ProteinKinematics(atom::Hierarchy mhd,
                    const atom::Residues& flexible_residues,
                    const ParticleIndexQuads& custom_dihedral_angles,
                    const std::vector<ProteinAngleType>& custom_dihedral_angle_types,    
                    atom::Atoms open_loop_bond_atoms = atom::Atoms(),
                    bool flexible_backbone = true,
                    bool flexible_side_chains = false);

 private:
  //! the actual construction is done here,
  //! see constructors for documentation
  /**
     initialize the protein kinematics object
     @param flexible_residues all reidues whose phi/psi angles are activated
     @param custom_dihedral_angles lists of four atoms that define a custom dihedral angle
     @param custom_dihedral_angle_types the types of all custom dihedral angles (a list of the same size)
     @param open_loop_bond_atoms TODO: document
     @param flexible_backbone are dihedral joints defined for the backbone
     @param flexible_side_chains are dihedral joints defined for the side chains
   */
  void init( const atom::Residues& flexible_residues,
             const ParticleIndexQuads& custom_dihedral_angles,
             const std::vector<ProteinAngleType>& custom_dihedral_angle_types,     
             atom::Atoms open_loop_bond_atoms,
             bool flexible_backbone,
             bool flexible_side_chains);

  void add_edges_to_rb_graph(const std::vector<atom::Atoms>& dihedral_angles);

  std::vector<atom::Atoms> quick_hack_converter(Model* m, const ParticleIndexQuads& piqs);

 public:
  /* Access methods */

  //! get phi angle associated with residue r
  double get_phi(const atom::Residue r) const {
    // TODO: what happens if user queries N' residue?
    return get_phi_joint(r)->get_angle();
  }

  //! get psi angle associated with residue r
  double get_psi(const atom::Residue r) const {
    // TODO: what happens if user queries C' residue?
    return get_psi_joint(r)->get_angle();
  }

  //! returns a list of all joints associated with the ProteinKinematics structure
  DihedralAngleRevoluteJoints get_joints() { return joints_; }

  DihedralAngleRevoluteJoints get_loop_joints() { return loop_joints_;
    // TODO: what are loop joints?
  }

  //! return joints sorted by BFS traversal from the
  //! root(s) of the kinematic structure
  DihedralAngleRevoluteJoints get_ordered_joints() {
    DihedralAngleRevoluteJoints ret;
    IMP_FOREACH(Joint *j, kf_->get_ordered_joints() ){
      ret.push_back(dynamic_cast<DihedralAngleRevoluteJoint*>(j));
    }
    return ret;
  }

  //! returns the kinematic forest associated with this ProteinKinematics
  //! object
  KinematicForest* get_kinematic_forest() { return kf_; }

  //! get all rigid bodies that were automatically
  //! generated in the tree
  core::RigidBodies get_rigid_bodies() { return rbs_; }

  // TODO: add chi

  /* Modifier methods */

  void set_phi(const atom::Residue r, double angle) {
    get_phi_joint(r)->set_angle(angle);
    kf_->update_all_external_coordinates();
  }

  void set_psi(const atom::Residue r, double angle) {
    get_psi_joint(r)->set_angle(angle);
    kf_->update_all_external_coordinates();
  }

  // TODO: add chi

  IMP_OBJECT_METHODS(ProteinKinematics);

  void build_topology_graph();

  void order_rigid_bodies(const std::vector<atom::Atoms>& dihedral_angles,
                          const std::vector<atom::Atoms>& phi_angles,
                          const std::vector<atom::Atoms>& psi_angles,
                          atom::Atoms open_loop_bond_atoms);

  //! mark specified dihedral angles as rotatable - remove the edges of the
  //! rotatable bonds from graph_, and add them to rb_graph
  void mark_rotatable_angles(const std::vector<atom::Atoms>& dihedral_angles);

  //! mark a single dihedral angle as rotatable - remove the edge of the
  //! rotatable bond from graph_, and add it to rb_graph
  void mark_rotatable_angle(const std::vector<atom::Atom>& dihedral_angle);

  //! automatically build rigid bodies for the protein kinematics structure
  void build_rigid_bodies();

  //void add_dihedral_joints(const std::vector<atom::Atoms>& dihedral_angles,
  //       const std::vector<ProteinAngleType>);

  void add_dihedral_joints(const std::vector<atom::Residue>& residues,
                           ProteinAngleType angle_type,
                           const std::vector<atom::Atoms>& dihedral_angles);

  void add_dihedral_joint(const atom::Residue r,
                          ProteinAngleType angle_type,
                          const atom::Atoms& atoms);

  void open_loop(atom::Atoms open_loop_bond_atoms);

  /* Joint access methods */
  DihedralAngleRevoluteJoint* get_phi_joint(const atom::Residue r) const {
    return (DihedralAngleRevoluteJoint*)joint_map_.get_joint(r, PHI);
  }

  DihedralAngleRevoluteJoint* get_psi_joint(const atom::Residue r) const {
    return (DihedralAngleRevoluteJoint*)joint_map_.get_joint(r, PSI);
  }

  DihedralAngleRevoluteJoint* get_other_joint(const atom::Residue r) const {
    return (DihedralAngleRevoluteJoint*)joint_map_.get_joint(r, OTHER);
  }

  //DihedralAngleRevoluteJoints get_joints(const atom::Residue r) const;

#ifndef IMP_DOXYGEN
  // A map between residue phi/psi and joints
  class AngleToJointMap {
   public:
    // Joint access
    Joint* get_joint(const atom::Residue r,
                     ProteinAngleType angle_type) const;

    // store Joint
    void add_joint(const atom::Residue r, ProteinAngleType angle_type,
                   Joint* joint);

   private:
    /* mapping to phi/psi/chi for a specific residue.
       the joints are stored using ProteinAngleType as an index */
    typedef std::vector<Joint*> ResidueJoints;
    /* mapping between residue and its joints */
    boost::unordered_map<ParticleIndex, ResidueJoints>
        residue_to_joints_;
  };
#endif  // IMP_DOXYGEN

 private:

  typedef boost::adjacency_list<
    boost::vecS, boost::vecS, boost::undirectedS > Graph;

  // protein hierarchy
  atom::Hierarchy mhd_;

  //! all atom particles in the protein
  ParticlesTemp atom_particles_;

  // topology graph: nodes = atoms, edges = bonds - includes all the atoms and non-rotatable bonds from the input protein hierarchy
  Graph graph_;

  // rigid bodies topology graph: node = rigid bodies, edges = joints - 
  Graph rb_graph_;

  // dfs order of rigid bodies
  std::vector<int> rb_order_, parents_;

  int largest_rb_;

  // mapping between atom ParticleIndex and node number in graph_ and rb_graph_
  boost::unordered_map<ParticleIndex, int> particle_index_to_node_map_, rb_particle_index_to_node_map_;

  Vector<ParticleIndex> node_to_particle_index_map_;

  // rigid bodies
  core::RigidBodies rbs_;

  // joints
  DihedralAngleRevoluteJoints joints_;

  PointerMember<kinematics::KinematicForest> kf_;

  // map between residue phi/psi/chis and joints
  AngleToJointMap joint_map_;

  boost::unordered_map<int, boost::unordered_map<int, Pointer<DihedralAngleRevoluteJoint> > > rigid_bodies_2_joint_map_;

  DihedralAngleRevoluteJoints loop_joints_;
};

IMP_OBJECTS(ProteinKinematics, ProteinKinematicsList);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_PROTEINKINEMATICS_H */
