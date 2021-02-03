/**
 *  \file IMP/spb/RigidBodyNewMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_RIGID_BODY_NEW_MOVER_H
#define IMPSPB_RIGID_BODY_NEW_MOVER_H

#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/core/rigid_bodies.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPSPBEXPORT RigidBodyNewMover : public core::MonteCarloMover {
 public:
  /** The rigid body is rotated and translated to move
      \param[in] d   the rigid body decorator
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  RigidBodyNewMover(core::RigidBody d, Float max_x_translation,
                    Float max_y_translation, Float max_z_translation,
                    Float max_rot);
  // IMP_MOVER(RigidBodyNewMover);

 protected:
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(RigidBodyNewMover);
  IMP_SHOWABLE(RigidBodyNewMover);

 private:
  algebra::Transformation3D last_transformation_;
  Float max_x_translation_;
  Float max_y_translation_;
  Float max_z_translation_;
  Float max_angle_;
  core::RigidBody d_;
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_RIGID_BODY_NEW_MOVER_H */
