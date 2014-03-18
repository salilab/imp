/**
 *  \file IMP/core/RigidBodyTunneler.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_TUNNELER_H
#define IMPCORE_RIGID_BODY_TUNNELER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "MonteCarloMover.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/core/rigid_bodies.h>
IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** Provided a number of entry points in cartesian space, this mover will
 * propose random translations of the rigid body from the closest entry point to
 * a randomly chosen other one. This way of moving thus satisfies detailed
 * balance. Entry points are relative to the center of the provided reference 
 * rigid body.
 * 
 * \see RigidBodyMover
 * \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyTunneler : public MonteCarloMover {
  algebra::Transformation3D last_transformation_;
  kernel::ParticleIndex pi_, ref_;
  algebra::Vector3Ds entries_;
  double move_probability_;

 public:
  /** Constructor
   * \param m the model
   * \param pi the rigidbody to move
   * \param pi the rigidbody reference
   * \param move_probability the prior probability to actually
   * move somewhere else
   */
  RigidBodyTunneler(kernel::Model *m, kernel::ParticleIndex pi,
          kernel::ParticleIndex ref, double move_probability=1.);

  /// add entry point in cartesian space
  void add_entry_point(algebra::Vector3D pt) { entries_.push_back(pt); }

 protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodyTunneler);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODY_TUNNELER_H */
