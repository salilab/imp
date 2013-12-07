/**
 * \file kinematics/KinematicForest.h
 * \brief Wrapper class for a kinematic forest (collection of trees)
          made of KinematicNode objects, interconnected by joints. This data
          structure allows for kinematic control of the tree and
          interconversion between internal and external coordinates.
 * \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_KINEMATIC_FOREST_H
#define IMPKINEMATICS_KINEMATIC_FOREST_H

#include "kinematics_config.h"
#include <IMP/kernel/Model.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/Joint.h>
#include <IMP/kinematics/TransformationJoint.h>
#include <IMP/base/Object.h>
#include <IMP/Decorator.h>
#include <IMP/base/set.h>
#include <IMP/base/exception.h>
#include <IMP/base/Object.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/warning_macros.h>
#include <IMP/atom/Hierarchy.h>
#include <queue>
#include <algorithm>

IMPKINEMATICS_BEGIN_NAMESPACE

/**
   An unified interface for defining and manipulating a kinematic
   structure over a model. The kinematic structure is defined as a
   forest over model particles at a certain hierarchy, such that a
   directed edge indicates the propagation of changes in internal
   coordinates (joint values) to values of external (cartesian)
   coordinates.

   \see Joint
 */
class IMPKINEMATICSEXPORT KinematicForest
    : public base::Object  // or kernel::ModelObject?
      {
 public:
  IMP_OBJECT_METHODS(KinematicForest);

  KinematicForest(kernel::Model* m);

  //     TODO: think about what foldtree scheme to use (star?),
  /**
     Builds a kinematic forest automatically from a hierarchy that
     contains rigid bodies and adds a ScoreState to the model, to
     make sure the internal and external coordinates are synced
     before model updating
  */
  KinematicForest(kernel::Model* m, IMP::atom::Hierarchy hierarchy);

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

  // rebuild tree (same topology but change directionality)
  void reset_root(IMP::kernel::Particle* new_root) {
    // TODO: implement
    IMP_NOT_IMPLEMENTED;
    IMP_UNUSED(new_root);
  }

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

  void update_all_external_coordinates() {
    if (is_external_coords_updated_) {
      return;
    }
    // tree BFS traversal from roots
    std::queue<KinematicNode> q;
    IMP::base::set<KinematicNode>::iterator it;
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

  // return joints sorted by BFS traversal
  Joints get_ordered_joints() const {
    Joints ret;
    // tree BFS traversal from roots
    std::queue<KinematicNode> q;
    IMP::base::set<KinematicNode>::iterator it;
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
     sets the corodinates of a particle, and makes sure that particles
     and joints in the tree will return correct external and internal
     coordinates

     @param rb a rigid body that was previously added to the tree
     @param c  new coordinates
  */
  void set_coordinates_safe(IMP::core::RigidBody rb, IMP::algebra::Vector3D c) {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were perviously added to it");
    rb.set_coordinates(c);
    mark_external_coordinates_changed();
  }

  /**
   */
  IMP::algebra::Vector3D get_coordinates_safe(IMP::core::RigidBody rb) const {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were perviously added to it");
    const_cast<KinematicForest*>(this)->update_all_external_coordinates();
    return rb.get_coordinates();
  }

  /**
   */
  bool get_is_member(IMP::core::RigidBody rb) const {
    kernel::Particle* p = rb.get_particle();
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
                        << " that were perviously added to it");
    const_cast<KinematicForest*>(this)->update_all_external_coordinates();
    return rb.get_reference_frame();
  }

  /**
     sets the reference frame of a rigid body, and makes sure that
     particles and joints in the tree will return correct external
     and internal coordinates when queried through the KinematicForest

     @param rb a rigid body that was previously added to the tree
     @param r  new reference frame
  */
  void set_reference_frame_safe(IMP::core::RigidBody rb,
                                IMP::algebra::ReferenceFrame3D r) {
    IMP_USAGE_CHECK(get_is_member(rb),
                    "A KinematicForest can only handle particles "
                        << " that were perviously added to it");
    rb.set_reference_frame(r);
    mark_external_coordinates_changed();
  }

  // TODO: handle derivatives, and safe getting / setting of them

 private:
  IMP::kernel::Particles get_children(IMP::kernel::Particle parent) const;

  IMP::kernel::Particle get_parent(IMP::kernel::Particle child) const;

#ifndef SWIG
  friend std::ostream& operator<<(std::ostream& s, const KinematicForest& kt);
#endif

 private:
  kernel::Model* m_;

  bool is_internal_coords_updated_;
  bool is_external_coords_updated_;

  /** the root nodes that serves as spatial anchor to the
      kinematic trees in the forest */
  IMP::base::set<KinematicNode> roots_;

  /** the set of nodes in the tree */
  IMP::base::set<KinematicNode> nodes_;

  Joints joints_;
};

IMP_OBJECTS(KinematicForest, KinematicForests);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_KINEMATIC_FOREST_H */
