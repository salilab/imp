/**
 *  \file VelocityScalingOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_VELOCITY_SCALING_OPTIMIZER_STATE_H
#define IMPCORE_VELOCITY_SCALING_OPTIMIZER_STATE_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Maintains temperature during molecular dynamics by velocity scaling.
/** This OptimizerState, when used with the MolecularDynamics optimizer,
    implements a simple thermostat by periodically rescaling the velocities.
    (Note that this results in discontinuous dynamics.)
    \see MolecularDynamics
    \deprecated Use atom::VelocityScalingOptimizerState instead
 */
class IMPCOREEXPORT VelocityScalingOptimizerState : public OptimizerState
{
 public:
  /** */
  VelocityScalingOptimizerState(const Particles &pis, Float temperature,
                                unsigned skip_steps);
  virtual ~VelocityScalingOptimizerState() {}

  IMP_OPTIMIZER_STATE(internal::version_info)

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

private:
  Particles pis_;
  Float temperature_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_VELOCITY_SCALING_OPTIMIZER_STATE_H */
