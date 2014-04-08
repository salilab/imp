/**
 *  \file IMP/atom/RemoveRigidMotionOptimizerState.h
 *  \brief Remove rigid rotation and translation during molecular dynamics.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes rigid translation and rotation from the particles.
class IMPATOMEXPORT RemoveRigidMotionOptimizerState : public OptimizerState {
 public:
  RemoveRigidMotionOptimizerState(kernel::Model *m,
                                  kernel::ParticleIndexesAdaptor pis);

  //! Set the particles to use.
  void set_particles(const kernel::Particles &pis) { pis_ = pis; }

  //! Remove rigid motion now
  void remove_rigid_motion() const;

  IMP_OBJECT_METHODS(RemoveRigidMotionOptimizerState);

 protected:
  virtual void do_update(unsigned int call) IMP_OVERRIDE;

 private:
  void remove_linear() const;
  void remove_angular() const;
  kernel::Particles pis_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(RemoveRigidMotionOptimizerState, RemoveRigidMotionOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H */
