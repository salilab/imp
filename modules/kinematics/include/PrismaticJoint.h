/**
 *  \file PrismaticJoint.h
 *  \brief a prismatic joint between rigid bodies as part of a kinematic tree
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_PRISMATIC_JOINT_H
#define IMPKINEMATICS_PRISMATIC_JOINT_H

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

/********************** PrismaticJoint ***************/

/**
   joint in which too rigid bodies may slide along a line
*/
class IMPKINEMATICSEXPORT PrismaticJoint : public Joint{
 public:
  /********* Constructors ********/
  /**
     Create a prismatic joint whose axis of translation is from a
     to b, which serve as witnesses for the joint transformation
     (a is associated with the parent and b with the child)
  */
 PrismaticJoint(IMP::core::RigidBody parent, IMP::core::RigidBody child,
                IMP::core::XYZ a, IMP::core::XYZ b);

  /**
     Create a prismatic joint whose axis of translation is between
     the reference framess of parent and child, who also
     serve as witnesses for the joint transformation
  */
 PrismaticJoint(IMP::core::RigidBody parent, IMP::core::RigidBody child);

 private:

 public:
  /************* Public getters / setters *************/
  /**
     gets the length of the prismatic joint, that is the length
     between the two witnesses
  */
  double get_length() const;

  /**
     sets the length of the prismatic joint to l, that is
     the length between the two witnesses set up upon construction
     (in a lazy fashion, without updating external coords).
     Updates the owner of the change in internal coordinates.

     @note it is assumed that l > 0, and for efficiency, runtime
           checks for this are not made in fast mode
  */
  void set_length(double l);

 protected:
  /************ General protected methods *************/

  /**
      Update the length and transformation of the prismatic joint
      based on the distance and relative orientation of the witnesses
      given upon construction.

     @note It is assumed that external coordinates are updated before
           calling this function.
  */
  virtual void update_joint_from_cartesian_witnesses();

 private:

  IMP::core::XYZ a_; // prismatic joint from point, associated with parent
  IMP::core::XYZ b_; // prismatic joint to point, associated with child
  double l_; // the length of the prismatic joint

};

IMP_OBJECTS(PrismaticJoint, PrismaticJoints);

IMPKINEMATICS_END_NAMESPACE

#endif  /* IMPKINEMATICS_PRISMATIC_JOINT_H */
