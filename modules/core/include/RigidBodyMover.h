/**
 *  \file IMP/core/RigidBodyMover.h
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_MOVER_H
#define IMPCORE_RIGID_BODY_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "MonteCarloMover.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of the rigid body is moved in two ways -
    the translation is moved with a ball of given size, and the
    rotation changed up to a given maximum angle. In both cases
    the probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyMover : public MonteCarloMover {
  algebra::Transformation3D last_transformation_;
  Float max_translation_;
  Float max_angle_;
  ParticleIndex pi_;

 public:
  //! Constructor. The given rigid body is rotated and translated.
  /** \param[in] m the Model
      \param[in] pi particle index of the rigid body
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  RigidBodyMover(Model *m, ParticleIndex pi,
                 Float max_translation, Float max_rotation);

  void set_maximum_translation(Float mt) {
    IMP_USAGE_CHECK(mt > 0, "Max translation must be positive");
    max_translation_ = mt;
  }

  void set_maximum_rotation(Float mr) {
    IMP_USAGE_CHECK(mr > 0, "Max rotation must be positive");
    max_angle_ = mr;
  }

  Float get_maximum_translation() const { return max_translation_; }

  Float get_maximum_rotation() const { return max_angle_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodyMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_MOVER_H */
