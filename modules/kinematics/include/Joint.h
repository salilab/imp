/**
 *  \file joints.h
 *  \brief functionality for defining a kinematic joint between rigid bodies
 *         as part of a kinematic tree
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_JOINT_H
#define IMPKINEMATICS_JOINT_H

#include <IMP/kinematics/kinematics_config.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/base/Object.h>
//#include <IMP/nullptr.h>
#include <IMP/exception.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/swig_macros.h>

IMPKINEMATICS_BEGIN_NAMESPACE

class KinematicForest;

/**
    Abstract class for joints between rigid bodies in a kinematic
    tree.
    // TODO: make abstract
*/
class IMPKINEMATICSEXPORT Joint
: public IMP::base::Object
{
  friend class KinematicForest;
  friend class CompositeJoint;

  IMP_OBJECT(Joint);

public:
  /**
     An abstract class for a joint between a parent and a child

     @param parent rigid body upstream of this joint
     @param child rigid body downstream of this joint
     @note we currently assume that a parent cannot be switched
   */
  Joint(IMP::core::RigidBody parent, IMP::core::RigidBody child);

  /***************** getter methods: ***************/

 public:

  KinematicForest* get_owner_kf() const{
    return owner_kf_;
  }

  /**
     returns the transformation of a vector from the child
     reference frame to the parent reference frame in a safe way
     (= after updating internal coordinates if needed)
  */
  // hack to make it compile on swig - this should be fixed as it might
  // cause some pathologies
IMP_NO_SWIG(virtual)
  const IMP::algebra::Transformation3D&
    get_transformation_child_to_parent() const;

  IMP::core::RigidBody  get_parent_node() const
    { return parent_; }

  IMP::core::RigidBody get_child_node() const
    { return child_; }


 protected:

    /**
       returns the transformation of a vector from the child
       reference frame to the parent reference frame, without any checks
       (= without updating internal coordinates even if needed)
    */
  // hack to make it compile on swig - this should be fixed as it might
  // cause some pathologies
  IMP_NO_SWIG(virtual)
 const IMP::algebra::Transformation3D&
    get_transformation_child_to_parent_no_checks() const {
    return tr_child_to_parent_;
  }


  /***************** setter methods: ***************/

 protected:

  /** this sets the kinematic forest that manages this joint,
      and also declares it as used (\see Object::set_was_used()
  */
  void set_owner_kf(KinematicForest* kf) {
    owner_kf_ = kf;
    Object::set_was_used(true);
  }

  /**
     Sets the transfromation from parent to child reference frame
     (without any checks that internal coords are updated, and without
      marking the owner internal coords as changed)
  */
  void set_transformation_child_to_parent_no_checks
    (IMP::algebra::Transformation3D transformation) {
    tr_child_to_parent_ = transformation;
  }


  /**************** general methods: **************/

  /**
     Updates the reference frame of the rigid body directly downstream
     of this joint
  */
  virtual void
    update_child_node_reference_frame() const;



  /**
     Updates the joint transformation based on external coordinates
     of 'witness' particles.

     @note It is assumed that external coordinates are updated before
           calling this function.
     @note Witness particles do not necessarily belong to the child or
           parent rigid bodes.
  */
  virtual void update_joint_from_cartesian_witnesses();

private:
  IMP::core::RigidBody parent_;
  IMP::core::RigidBody child_;
  IMP::algebra::Transformation3D tr_child_to_parent_;
  KinematicForest* owner_kf_; // the tree that manages updates to this joint
};

IMP_OBJECTS(Joint, Joints);

IMPKINEMATICS_END_NAMESPACE

#endif  /* IMPKINEMATICS_JOINT_H */
