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
#include <IMP/symmetry/RigidBodyMover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
//#include <IMP/core/rigid_bodies.h>
//#include <IMP/core/mover_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT PbcBoxedRigidBodyMover: public symmetry:RigidBodyMover
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
                 algebra::Transformation3Ds transformations,
                 Particle *px, Particle *py, Particle *pz);

protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;

  IMP_OBJECT_METHODS(PbcBoxedRigidBodyMover);

private:
  // particles for scaling
  IMP::base::PointerMember<kernel::Particle> px_;
  IMP::base::PointerMember<kernel::Particle> py_;
  IMP::base::PointerMember<kernel::Particle> pz_;

  algebra::Vector3D get_vector(algebra::Vector3D center);
  algebra::Transformation3D get_transformation(algebra::Transformation3D trans);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_PBC_BOXED_RIGID_BODY_MOVER_H */
