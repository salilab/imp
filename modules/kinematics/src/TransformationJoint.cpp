/**
 *  \file TransformationJoint.cpp
 *  \brief a kinematic joints between rigid bodies that allows any
 *         transformation
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/TransformationJoint.h>
#include <IMP/base/Object.h>
#include <IMP/base/exception.h>
#include <IMP/algebra/Transformation3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE

/********************** Transformation Joint ***************/

TransformationJoint::TransformationJoint(IMP::core::RigidBody parent,
                                         IMP::core::RigidBody child)
    : Joint(parent, child) {}

// Sets the transformation from parent to child
void TransformationJoint::set_transformation_child_to_parent(
    IMP::algebra::Transformation3D transformation) {
  if (get_owner_kf()) {
    get_owner_kf()->update_all_internal_coordinates();
  }
  Joint::set_transformation_child_to_parent_no_checks(transformation);
  if (get_owner_kf()) {
    get_owner_kf()->mark_internal_coordinates_changed();
  }
}

IMPKINEMATICS_END_NAMESPACE
