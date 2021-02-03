/**
 * \file IMP/kinematics/KinematicForest.h
 * \brief Define and manipulate a kinematic structure over a model.
 * \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_KINEMATIC_FOREST_H
#define IMPKINEMATICS_KINEMATIC_FOREST_H

#include "kinematics_config.h"
#include <IMP/Model.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/Joint.h>
#include <IMP/kinematics/TransformationJoint.h>
#include <IMP/Object.h>
#include <IMP/Decorator.h>
#include <boost/unordered_set.hpp>
#include <IMP/exception.h>
#include <IMP/Object.h>
#include <IMP/check_macros.h>
#include <IMP/warning_macros.h>
#include <IMP/atom/Hierarchy.h>
#include <queue>
#include <algorithm>

IMPKINEMATICS_BEGIN_NAMESPACE

//! Define and manipulate a kinematic structure over a model.
/** The kinematic structure is defined as a
   forest over model particles at a certain hierarchy, such that a
   directed edge indicates the propagation of changes in internal
   coordinates (joint values) to values of external (Cartesian)
   coordinates.

   \see Joint
 */
class IMPKINEMATICSEXPORT KinematicForest
    : public Object  // or ModelObject?
      {
 public:
  IMP_OBJECT_METHODS(KinematicForest);

  KinematicForest(Model* m);

  //     TODO: think about what foldtree scheme to use (star?),
  /**
     Builds a kinematic forest automatically from a hierarchy that
     contains rigid bodies and adds a ScoreState to the model, to
     make sure the internal and external coordinates are synced
     before model updating
  */
  KinematicForest(Model* m, IMP::atom::Hierarchy hierarchy);

  /**
     Adds a kinematic edge between parent and child,
     using a TransformationJoint between them, and
     decorating them as KinematicNodes if needed.
  */
  // NOTE: must have root on first call
  // TODO: verify parent_rb is in tree
  Joint* add_edge(IMP::core::RigidBody parent, IMP::core::RigidBody child) {
    // create joint and associate it with parent and child
    IMP_NEW(TransformationJoint, joint, (parent, child));
    add_edge(joint);
    return joint;
  }

  /**
     Adds a kinematic edge between the joint parent and child
     rigid bodies, decorating them as KinematicNodes if needed.
     The joint becomes owned by this KinematicForest, such that
     changes to the joint are synchronized with the KinematicForest

     @note it is assumed that neither the joint or the rigid bodies in it
           were previously added to a kinematic forest (might change in
           future IMP versions)
  */
  void add_edge(Joint* joint);

  /**
     adds edges between each pair of consecutive rigid bodies in the list
     rbs, using default TransformationJoint joints (transforming from one
     rigid body to the next)

     @param rbs list of n consecutive rigid bodies
  */
  void add_rigid_bodies_in_chain(IMP::core::RigidBodies rbs) {
    for (int i = 0; i < (int)rbs.size() - 1; i++) {
      add_edge(rbs[i], rbs[i + 1]);
    }
  }

  //! rebuild tree (same topology but change directionality)
  void reset_root(IMP::Particle* new_root) {
    // TODO: implement
    IMP_NOT_IMPLEMENTED;
    IMP_UNUSED(new_root);
  }

  //! updated internal coordinates in the forest based on the current cartesian
  //! coordinates and the architechture of joints in the tree
  void update_all_internal_coordinates() {
    IMP_LOG(VERBOSE, "updating internal coords needed?" << std::endl);
    if (is_internal_coords_updated_) {
      return;
    }
    IMP_LOG(VERBOSE, "updating!" << std::endl);
    for (unsigned int i = 0; i < joints_.size(); i++) {
      joints_[i]->update_joint_from_cartesian_witnesses();
    }
    is_internal_coords_updated_ = true;
  }

  //! update all external coordinates of particles in the forest
  //! based on their internal coordinates
  void update_all_external_coordinates() {
    if (is_external_coords_updated_) {
      return;
    }
    // tree BFS traversal from roots
    std::queue<KinematicNode> q;
    boost::unordered_set<KinematicNode>::iterator it;
    for (it = roots_.begin(); it != roots_.end(); it++) {
      q.push(*it);
    }
    while (!q.empty()) {
      KinematicNode n = q.front();
      q.pop();
      JointsTemp out_joints = n.get_out_joints();
      for (unsigned int i = 0; i < out_joints.size(); i++) {
        Joint* joint_i = out_joints[i];
        // TODO: add and implement to joint
        joint_i->update_child_node_reference_frame();
        q.push(KinematicNode(joint_i->get_child_node()));
      }
    }
    is_external_coords_updated_ = true;
  }

  //! return joints sorted by BFS traversal
  Joints get_ordered_joints() const {
    Joints ret;
    // tree BFS traversal from roots
    std::queue<KinematicNode> q;
    boost::unordered_set<KinematicNode>::iterator it;
    for (it = roots_.begin(); it != roots_.end(); it++) q.push(*it);

    while (!q.empty()) {
      KinematicNode n = q.front();
      q.pop();
      if (n.get_in_joint() != nullptr) ret.push_back(n.get_in_joint());

      JointsTemp out_joints = n.get_out_joints();
      for (unsigned int i = 0; i < out_joints.size(); i++) {
        Joint* joint_i = out_joints[i];
        q.push(KinematicNode(joint_i->get_child_node()));
      }
    }
    return ret;
  }

  //! apply a rigid body transformation to the entire forest
  //! safelt, such that the forest will return correct external
  //! and internal coordinates if queries through get_coordinates_safe()
  //! or get_reference_frame_safe(), or after updating with
  //! update_all_external_coordinates() or update_all_internal_coordinates(),
  //! respectively.
  void transform_safe(IMP::algebra::Transformation3D tr){
    IMP_FOREACH(KinematicNode root, roots_){
      IMP::core::transform(root, tr);
      mark_internal_coordinates_changed(); // technically, roots reference frames is a part of the internal tree coordinates, so external coordinates will need to be updated at some point
    }
  }

  /**
     notifies the tree that joint (internal) coordinates
     have changed and therefore external coordinates are not
     up to date
  */
  void mark_internal_coordinates_changed() {
    is_external_coords_updated_ = false;
  }

  /**
     notifies the tree that external Cartesian coordinates
     have changed and therefore internal (joint) coordinates are not
     up to date
  */
  void mark_external_coordinates_changed() {
    is_internal_coords_updated_ = false;
  }

  /**
     sets the coordinates of a particle, and makes sure that particles
     and joints in the tree will return correct external and internal
     coordinates

     @param rb a rigid body that was previously added to the tree
     @param c  new coordinates
  */
  void set_coordinates_safe(IMP::core::RigidBody rb, IMP::algebra::Vector3D c) {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were previously added to it");
    rb.set_coordinates(c);
    mark_external_coordinates_changed();
  }

  /**
   */
  IMP::algebra::Vector3D get_coordinates_safe(IMP::core::RigidBody rb) const {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were previously added to it");
    const_cast<KinematicForest*>(this)->update_all_external_coordinates();
    return rb.get_coordinates();
  }

  /**
   */
  bool get_is_member(IMP::core::RigidBody rb) const {
    Particle* p = rb.get_particle();
    return KinematicNode::get_is_setup(p) &&
           nodes_.find(KinematicNode(p)) != nodes_.end();
  }

  // TODO: do we want to add safe access to rigid body members?

  /**
   */
  IMP::algebra::ReferenceFrame3D get_reference_frame_safe(
      IMP::core::RigidBody rb) const {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were previously added to it");
    const_cast<KinematicForest*>(this)->update_all_external_coordinates();
    return rb.get_reference_frame();
  }

  /**
     sets the reference frame of a rigid body, and makes sure that
     particles and joints in the tree will return correct
     internal coordinates when queried directly through the
     KinematicForest

     @param rb a rigid body that was previously added to the tree
     @param r  new reference frame
  */
  void set_reference_frame_safe(IMP::core::RigidBody rb,
                                IMP::algebra::ReferenceFrame3D r) {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were previously added to it");
    rb.set_reference_frame(r);
    mark_external_coordinates_changed();
  }

  //! apply a rigid body transformation to the entire forest safely
  /** Apply a rigid body transformation to the entire forest
      safely, such that the forest will return correct external
      and internal coordinates if queries through get_coordinates_safe()
      or get_reference_frame_safe(), or after updating with
      update_all_external_coordinates() or update_all_internal_coordinates(),
      respectively.
  */
  void apply_transform_safely(IMP::algebra::Transformation3D tr)
  {
    IMP_FOREACH(KinematicNode root, roots_){
      IMP::core::transform(root, tr);
      mark_internal_coordinates_changed(); // technically, roots reference frames is a part of the internal tree coordinates, so external coordinates will need to be updated at some point
    }
  }

  // TODO: handle derivatives, and safe getting / setting of them

 private:
  IMP::Particles get_children(IMP::Particle parent) const;

  IMP::Particle get_parent(IMP::Particle child) const;

#ifndef SWIG
  friend std::ostream& operator<<(std::ostream& s, const KinematicForest& kt);
#endif

 private:
  Model* m_;

  bool is_internal_coords_updated_;
  bool is_external_coords_updated_;

  /** the root nodes that serves as spatial anchor to the
      kinematic trees in the forest */
  boost::unordered_set<KinematicNode> roots_;

  /** the set of nodes in the tree */
  boost::unordered_set<KinematicNode> nodes_;

  Joints joints_;
};

IMP_OBJECTS(KinematicForest, KinematicForests);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_KINEMATIC_FOREST_H */
