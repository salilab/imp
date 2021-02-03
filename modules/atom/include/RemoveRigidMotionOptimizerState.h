/**
 *  \file IMP/atom/RemoveRigidMotionOptimizerState.h
 *  \brief Remove rigid rotation and translation during molecular dynamics.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes rigid translation and rotation from the particles.
class IMPATOMEXPORT RemoveRigidMotionOptimizerState : public OptimizerState {
 public:
  RemoveRigidMotionOptimizerState(Model *m,
                                  ParticleIndexesAdaptor pis);

  //! Set the particles to use.
  void set_particles(const Particles &pis) { pis_ = pis; }

  //! Remove rigid motion now
  void remove_rigid_motion() const;

  IMP_OBJECT_METHODS(RemoveRigidMotionOptimizerState);

 protected:
  virtual void do_update(unsigned int call) IMP_OVERRIDE;

 private:
  void remove_linear() const;
  void remove_angular() const;
  Particles pis_;
};

IMP_OBJECTS(RemoveRigidMotionOptimizerState, RemoveRigidMotionOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H */
