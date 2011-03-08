/**
 *  \file atom/LangevinThermostatOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics using
 *         a Langevin thermostat.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H
#define IMPATOM_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H

#include "atom_config.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Maintains temperature during molecular dynamics.
/** The thermostat scales velocities using the algorithm described in
    G. Bussi and M. Parrinello "Accurate sampling using Langevin dynamics",
    Phys. Rev. E 75, 056707 (2007)
 */
class IMPATOMEXPORT LangevinThermostatOptimizerState : public OptimizerState
{
 public:
  LangevinThermostatOptimizerState(const Particles &pis,
                                   double temperature,
                                   double time_friction,
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
  void rescale_velocities() const;

  IMP_OPTIMIZER_STATE(LangevinThermostatOptimizerState);

private:
  Particles pis_;
  double temperature_;
  double time_friction_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(LangevinThermostatOptimizerState,LangevinThermostatOptimizerStates);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H */
