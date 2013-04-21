/**
 *  \file CompositeJoint.h
 *  \brief a joint composed of several joints, applied on the same
 *         pair of rigid bodies
 *  \authors Dina Schneidman, Barak Raveh
 *

 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_COMPOSITE_JOINT_H
#define IMPKINEMATICS_COMPOSITE_JOINT_H

#include "kinematics_config.h"
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/Joint.h>
#include <IMP/base/Object.h>
//#include <IMP/compatibility/nullptr.h>
#include <IMP/exception.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/base/check_macros.h>

IMPKINEMATICS_BEGIN_NAMESPACE

class KinematicForest;

/********************** CompositeJoint ***************/

/**
    A joint that combines several inner joints, acting on the same
    pair of rigid bodies
*/
class IMPKINEMATICSEXPORT CompositeJoint : public Joint
{
 public:
  /**
     Constructs a composite joint between parent and child,
     with the specified list of inner joints connecting them.

     @param parent rigid body upstream of this joint
     @param child rigid body downstream of this joint
     @param[in]  joints The list of inner joints connecting the two
                        rigid bodies. These will be applied by their
                        specified order in the list, from the parent
                        rigid body to the child rigid body. It is
                        assumed all these joints share the same
                        parent and child as the composite joint.
  */
  CompositeJoint
    (IMP::core::RigidBody parent,
     IMP::core::RigidBody child,
     Joints joints = Joints() );

  /**
     adds a joint at the end of the list of joints (closest to the child
     rigid body)
     @note the joint must have the same parent and child as the composite
           joint
   */
  void add_downstream_joint(Joint* j) {
    IMP_ALWAYS_CHECK( j->get_parent_node() == this->get_parent_node() &&
                      j->get_child_node() == this->get_child_node(),
                      "inner joint within a composite joint must have"
                      "the same parent and child rigid body nodes",
                      ValueException);
    joints_.push_back(j);
  }

  /**
     adds a joint at the front of the list of joints (closest to the parent
     rigid body)
     @note the joint must have the same parent and child as the composite
           joint
   */
  void add_upstream_joint(Joint* j) {
    IMP_ALWAYS_CHECK( j->get_parent_node() == this->get_parent_node() &&
                      j->get_child_node() == this->get_child_node(),
                      "inner joint within a composite joint must have"
                      << "the same parent and child rigid body nodes",
                      ValueException);
    joints_.insert(joints_.begin(), j);
  }

  /**
     Sets the list of inner joints instead of the existing one,

     @param joints the new joints, ordered from the parent rigid body
                   downstream to the child rigid body.

     @note All joints must have the same parent and child as the composite
           joint
     @note This invalidates all existing inner joints if any, detaching
     them from their current KinematicForest owner if it exists.
  */
  void set_joints(Joints joints);

  /**
     returns the list of inner joints, ordered from the parent
     rigid body downstream to the child rigid body
  */
  const Joints& get_inner_joints() const {
    return joints_;
  }

#ifndef SWIG
  /**
     returns the list of inner joints, ordered from the parent
     rigid body downstream to the child rigid body
  */
  Joints& get_inner_joints() {
    return joints_;
  }
#endif


 protected:
  /**
     update the child node reference frame by applying all the
     inner joints
   */
  virtual void update_child_node_reference_frame() const;


  /**
    Updates all inner joints value, and the overall transformation
    resulting from their combinations, based on the external coordinates
    of the witnesses to each of the inner joints.

    @note It is assumed that external coordinates are updated before
          calling this function.
  */

  virtual void update_joint_from_cartesian_witnesses();

 private:
  Joints joints_; // list of inner joints
};

IMP_OBJECTS(CompositeJoint, CompositeJoints);

IMPKINEMATICS_END_NAMESPACE

#endif  /* IMPKINEMATICS_COMPOSITE_JOINT_H */
