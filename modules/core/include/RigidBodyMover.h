/**
 *  \file RigidBodyMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_MOVER_H
#define IMPCORE_RIGID_BODY_MOVER_H

#include "core_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyMover : public Mover
{
public:
  /** The rigid body is rotated and translated to move
      \param[in] d   the rigid body decorator
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  RigidBodyMover(RigidBody d,Float max_translation,
                 Float max_rotation);
  void set_maximum_translation(Float mt) {
    IMP_USAGE_CHECK(mt > 0, "Max translation must be positive");
    max_translation_=mt;
  }
  void set_maximum_rotation(Float mr) {
    IMP_USAGE_CHECK(mr > 0, "Max rotation must be positive");
    max_angle_=mr;
  }
  Float get_maximum_translation() const {
    return max_translation_;
  }
  Float get_maximum_rotation() const {
    return max_angle_;
  }
  IMP_MOVER(RigidBodyMover);
private:
  algebra::Transformation3D last_transformation_;
  Float max_translation_;
  Float max_angle_;
  RigidBody d_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_MOVER_H */
