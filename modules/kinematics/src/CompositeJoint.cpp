/**
 *  \file CompositeJoint.cpp
 *  \brief a joint composed of several joints, applied on the same
 *         pair of rigid bodies
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */


#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/CompositeJoint.h>
#include <IMP/Object.h>
#include <IMP/base/nullptr.h>
#include <IMP/exception.h>
#include <IMP/algebra/Transformation3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE



/********************** CompositeJoint ***************/

CompositeJoint::CompositeJoint
(IMP::core::RigidBody parent, IMP::core::RigidBody child, Joints joints )
  : Joint(parent, child)
{
  set_joints( joints );
  update_joint_from_cartesian_witnesses();
}


void
CompositeJoint::update_child_node_reference_frame() const
{
  // TODO: make this efficient - augment all joint transformation
  //       in global coordinates instead of doing it one by one?
  using namespace IMP::algebra;

  for(int i = (int)joints_.size() - 1; i >= 0; i++) {
    joints_[i]->update_child_node_reference_frame();
  }
  Joint::update_child_node_reference_frame();
}


void CompositeJoint::set_joints(Joints joints) {
  for(unsigned int i = 0; i < joints_.size(); i++){
    joints_[i]->set_owner_kf( nullptr );
  }
  // add new joints
  for(unsigned int i = 0; i < joints.size(); i++){
    add_downstream_joint( joints[i] );
  }
}


void CompositeJoint::update_joint_from_cartesian_witnesses() {
  for(unsigned int i = 0; i < joints_.size(); i++){
    joints_[i]->update_joint_from_cartesian_witnesses();
  }
  Joint::update_joint_from_cartesian_witnesses();
}


IMPKINEMATICS_END_NAMESPACE
