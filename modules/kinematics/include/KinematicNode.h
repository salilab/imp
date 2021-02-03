/**
 *  \file IMP/kinematics/KinematicNode.h
 *  \brief functionality for defining nodes on a kinematic chain
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_KINEMATIC_NODE_H
#define IMPKINEMATICS_KINEMATIC_NODE_H

#include "kinematics_config.h"
#include <IMP/core/rigid_bodies.h>
#include <IMP/kinematics/Joint.h>

#include <IMP/exception.h>

IMPKINEMATICS_BEGIN_NAMESPACE

class KinematicForest;

// TODO: check for cycles
// TODO: privatize most methods INCLUDING constructor

//! A rigid body that is connected by a joint to other rigid bodies
class IMPKINEMATICSEXPORT KinematicNode : public IMP::core::RigidBody {
  friend class KinematicForest;

  static void do_setup_particle(Model* m, ParticleIndex p,
                                KinematicForest* owner,
                                Joint* in_joint = nullptr,
                                Joints out_joints = Joints());

 public:
  IMP_DECORATOR_METHODS(KinematicNode, IMP::core::RigidBody);
  IMP_DECORATOR_SETUP_1(KinematicNode, KinematicForest*, owner);
  IMP_DECORATOR_SETUP_2(KinematicNode, KinematicForest*, owner, Joint*,
                        in_joint);
  IMP_DECORATOR_SETUP_3(KinematicNode, KinematicForest*, owner, Joint*,
                        in_joint, Joints, out_joints);

  //! Return true if the particle is a kinematic node
  inline static bool get_is_setup(Model* m, ParticleIndex pi);

 private:
  //! returns the kinematic forest associated with this node
  KinematicForest* get_owner();

  //! return nullptr if does not have incoming joint
  inline Joint* get_in_joint();

  //! returns list of outcoming joints
  inline JointsTemp get_out_joints();

  void set_out_joints(Joints in);

  void add_out_joint(Joint* j);

  void set_in_joint(Joint* j);

  static ObjectKey get_owner_key() {
    static ObjectKey k("kinematics__kinematic_node_owner");
    return k;
  }

  static ObjectKey get_in_joint_key() {
    static ObjectKey k("kinematics__kinematic_node_in_joint");
    return k;
  }

  static ObjectsKey get_out_joints_key() {
    static ObjectsKey k("kinematics__kinematic_node_out_joint");
    return k;
  }
};

/************** inlines ***********/

bool KinematicNode::get_is_setup(Model* m, ParticleIndex pi) {
  return m->get_has_attribute(get_owner_key(), pi);
}

//! return nullptr if does not have incoming joint
Joint* KinematicNode::get_in_joint() {
  if (!get_model()->get_has_attribute(get_in_joint_key(),
                                      get_particle_index())) {
    return nullptr;
  }
  Object* obj =
      get_model()->get_attribute(get_in_joint_key(), get_particle_index());
  return static_cast<Joint*>(obj);
}

//! returns list of outcoming joints, or empty list if attribute
//! does not exist
JointsTemp KinematicNode::get_out_joints() {
  JointsTemp joints;
  if (!get_model()->get_has_attribute(get_out_joints_key(),
                                      get_particle_index())) {
    return joints;
  }
  Objects objs =
      get_model()->get_attribute(get_out_joints_key(), get_particle_index());
  for (unsigned int i = 0; i < objs.size(); i++) {
    Object* o = objs[i];
    Joint* j = static_cast<Joint*>(o);
    joints.push_back(j);
  }
  return joints;
}

// IMP_DECORATORS_DEF(KinematicNode, KinematicNodes);
IMP_DECORATORS(KinematicNode, KinematicNodes, IMP::core::RigidBody);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_KINEMATIC_NODE_H */
