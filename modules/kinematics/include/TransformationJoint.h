/**
 *  \file TransformationJoint.h
 *  \brief a kinematic joints between rigid bodies that allows any
 *         transformation
 *  \authors Dina Schneidman, Barak Raveh
 *

 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_TRANSFORMATION_JOINT_H
#define IMPKINEMATICS_TRANSFORMATION_JOINT_H

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



/********************** TransformationJoint ***************/


/* /\** A joint with a completely non-constrained transformation */
/*     between parent and child nodes reference frames */
/* *\/ */
class  IMPKINEMATICSEXPORT
TransformationJoint : public Joint{
 public:
  TransformationJoint(IMP::core::RigidBody parent,
                      IMP::core::RigidBody child);

  /**
     Sets the transfromation from parent to child reference frame,
     in a safe way - that is, after updating all intrnal coordinates
     from external if needed, and marking the owner internal coordinates
     as changed.
  */
  void set_transformation_child_to_parent
    (IMP::algebra::Transformation3D transformation);


};

IMP_OBJECTS(TransformationJoint, TransformationJoints);

IMPKINEMATICS_END_NAMESPACE

#endif  /* IMPKINEMATICS_TRANSFORMATION_JOINT_H */
