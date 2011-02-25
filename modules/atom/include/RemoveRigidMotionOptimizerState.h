/**
 *  \file atom/RemoveRigidMotionOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H

#include "atom_config.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes new translationa and rotations from the set of points..
/**  */
class IMPATOMEXPORT RemoveRigidMotionOptimizerState : public OptimizerState
{
 public:
  RemoveRigidMotionOptimizerState(const Particles &pis,
                                  unsigned skip_steps);

  //! Set the number of update calls to skip between rescaling.
  void set_skip_steps(unsigned skip_steps) {
    skip_steps_ = skip_steps;
  }

  //! Set the particles to use.
  void set_particles(const Particles &pis) {
    pis_=pis;
  }

  //! Rescale the velocities now
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

IMP_OBJECTS(RemoveRigidMotionOptimizerState,RemoveRigidMotionOptimizerStates);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_REMOVE_RIGID_MOTION_OPTIMIZER_STATE_H */
