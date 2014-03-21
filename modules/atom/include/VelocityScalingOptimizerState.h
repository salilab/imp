/**
 *  \file IMP/atom/VelocityScalingOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_VELOCITY_SCALING_OPTIMIZER_STATE_H
#define IMPATOM_VELOCITY_SCALING_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Maintains temperature during molecular dynamics by velocity scaling.
/** This OptimizerState, when used with the MolecularDynamics optimizer,
    implements a simple thermostat by periodically rescaling the velocities.
    (Note that this results in discontinuous dynamics.)
    \see MolecularDynamics
 */
class IMPATOMEXPORT VelocityScalingOptimizerState : public OptimizerState {
 public:
  VelocityScalingOptimizerState(kernel::Model *m,
                                kernel::ParticleIndexesAdaptor pis,
                                double temperature);

  //! Set the particles to use.
  void set_particles(const kernel::Particles &pis) { pis_ = pis; }

  //! Set the temperature to use.
  void set_temperature(Float temperature) { temperature_ = temperature; }

  //! Rescale the velocities now
  void rescale_velocities() const;

  IMP_OBJECT_METHODS(VelocityScalingOptimizerState);

 protected:
  virtual void do_update(unsigned int call) IMP_OVERRIDE;

 private:
  kernel::Particles pis_;
  Float temperature_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(VelocityScalingOptimizerState, VelocityScalingOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_VELOCITY_SCALING_OPTIMIZER_STATE_H */
