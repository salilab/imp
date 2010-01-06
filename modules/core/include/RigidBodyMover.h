/**
 *  \file RigidBodyMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_MOVER_H
#define IMPCORE_RIGID_BODY_MOVER_H

#include "config.h"
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
  void reject_move();
  void accept_move();
  void propose_move(Float f);
  void show(std::ostream&out= std::cout) const {
    out << "RigidTransformationMover " << std::endl;
  }

  VersionInfo get_version_info() const {
    return get_module_version_info();
  }
  IMP_REF_COUNTED_DESTRUCTOR(RigidBodyMover)
private:
  algebra::Transformation3D last_transformation_;
  Float max_translation_;
  Float max_angle_;
  RigidBody d_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_MOVER_H */
