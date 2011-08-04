/**
 *  \file PbcBoxedRigidBodyMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_PBC_BOXED_RIGID_BODY_MOVER_H
#define IMPMEMBRANE_PBC_BOXED_RIGID_BODY_MOVER_H

#include "membrane_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
IMPMEMBRANE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT PbcBoxedRigidBodyMover : public core::Mover
{
public:
  /** The rigid body is rotated and translated to move
      \param[in] d   the rigid body decorator
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  PbcBoxedRigidBodyMover(core::RigidBody d, Particles ps,
                 Float max_translation,
                 Float max_rotation, algebra::Vector3Ds centers,
                 algebra::Transformation3Ds transformations);
  IMP_MOVER(PbcBoxedRigidBodyMover);
private:
  algebra::Transformation3D last_transformation_;
  algebra::Vector3Ds oldcoords_;
  Float max_translation_;
  Float max_angle_;
  algebra::Vector3Ds centers_;
  algebra::Transformation3Ds transformations_;
  core::RigidBody d_;
  Particles ps_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_PBC_BOXED_RIGID_BODY_MOVER_H */
