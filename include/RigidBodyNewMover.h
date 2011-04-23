/**
 *  \file RigidBodyNewMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_RIGID_BODY_NEW_MOVER_H
#define IMPMEMBRANE_RIGID_BODY_NEW_MOVER_H

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
class IMPMEMBRANEEXPORT RigidBodyNewMover : public core::Mover
{
public:
  /** The rigid body is rotated and translated to move
      \param[in] d   the rigid body decorator
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  RigidBodyNewMover(core::RigidBody d, Float max_x_translation,
                 Float max_y_translation, Float max_z_translation,
                 Float max_a_rot, Float max_b_rot, Float max_c_rot);
  void reset_move();
  void propose_move(Float f);
  IMP_OBJECT(RigidBodyNewMover);
private:
  algebra::Transformation3D last_transformation_;
  Float max_x_translation_;
  Float max_y_translation_;
  Float max_z_translation_;
  Float max_a_angle_;
  Float max_b_angle_;
  Float max_c_angle_;
  core::RigidBody d_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_RIGID_BODY_NEW_MOVER_H */
