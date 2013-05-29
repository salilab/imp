/**
 *  \file IMP/atom/RemoveRigidMotionOptimizerState.h
 *  \brief Remove rigid rotation and translation during molecular dynamics.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes rigid translation and rotation from the particles.
class IMPATOMEXPORT RemoveRigidMotionOptimizerState : public OptimizerState {
 public:
  RemoveRigidMotionOptimizerState(const ParticlesTemp &pis,
                                  unsigned skip_steps);

  //! Set the number of update calls to skip between removals.
  void set_skip_steps(unsigned skip_steps) { skip_steps_ = skip_steps; }

  //! get the number of update calls to skip between rescaling.
  unsigned int get_skip_steps() { return skip_steps_; }

  //! Set the particles to use.
  void set_particles(const Particles &pis) { pis_ = pis; }

  //! Remove rigid motion now
  void remove_rigid_motion() const;

  IMP_OPTIMIZER_STATE(RemoveRigidMotionOptimizerState);

 private:
  void remove_linear() const;
  void remove_angular() const;
  Particles pis_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(RemoveRigidMotionOptimizerState, RemoveRigidMotionOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H */
