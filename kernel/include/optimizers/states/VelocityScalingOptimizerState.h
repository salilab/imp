/**
 *  \file VelocityScalingOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VELOCITY_SCALING_OPTIMIZER_STATE_H
#define __IMP_VELOCITY_SCALING_OPTIMIZER_STATE_H

#include "../../IMP_config.h"
#include "../../base_types.h"
#include "../../OptimizerState.h"
#include "../../internal/kernel_version_info.h"

IMP_BEGIN_NAMESPACE

//! Maintains temperature during molecular dynamics by velocity scaling.
/** This OptimizerState, when used with the MolecularDynamics optimizer,
    implements a simple thermostat by periodically rescaling the velocities.
    (Note that this results in discontinuous dynamics.)
 */
class IMPDLLEXPORT VelocityScalingOptimizerState : public OptimizerState
{
 public:
  VelocityScalingOptimizerState(const Particles &pis, Float temperature,
                                unsigned skip_steps);
  virtual ~VelocityScalingOptimizerState() {}

  IMP_OPTIMIZER_STATE(internal::kernel_version_info)

  //! Set the number of update calls to skip between rescaling.
  void set_skip_steps(unsigned skip_steps) {
    skip_steps_ = skip_steps;
  }

  //! Set the particles to use.
  void set_particles(const Particles &pis) {
    pis_=pis;
  }

  //! Set the temperature to use.
  void set_temperature(Float temperature) {
    temperature_ = temperature;
  }

  //! Rescale the velocities now
  void rescale_velocities() const;

protected:
  Particles pis_;
  Float temperature_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_END_NAMESPACE

#endif  /* __IMP_VELOCITY_SCALING_OPTIMIZER_STATE_H */
