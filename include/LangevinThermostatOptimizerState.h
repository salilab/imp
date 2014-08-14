/**
 *  \file membrane/LangevinThermostatOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics using
 *         a Langevin thermostat.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H
#define IMPMEMBRANE_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H

#include "membrane_config.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>
//#include <IMP/optimizer_state_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Maintains temperature during molecular dynamics.
/** The thermostat scales velocities using the algorithm described in
    G. Bussi and M. Parrinello "Accurate sampling using Langevin dynamics",
    Phys. Rev. E 75, 056707 (2007)
 */
class IMPMEMBRANEEXPORT LangevinThermostatOptimizerState : public OptimizerState
{
 public:
  LangevinThermostatOptimizerState(const ParticlesTemp &pis,
                                   double temperature,
                                   double gamma);

  //! Set the particles to use.
  void set_particles(const Particles &pis) {
    pis_=pis;
  }

  double get_temperature() {
      return temperature_;
  }

  double get_gamma() {
      return gamma_;
  }

  void set_temperature(double temperature) {
      temperature_ = temperature;
  }

  void set_gamma(double gamma) {
      gamma_ = gamma;
  }

  //! Rescale the velocities now
  void rescale_velocities() const;

  IMP_OBJECT_METHODS(LangevinThermostatOptimizerState);

private:
  Particles pis_;
  double temperature_;
  double gamma_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(LangevinThermostatOptimizerState,LangevinThermostatOptimizerStates);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_LANGEVIN_THERMOSTAT_OPTIMIZER_STATE_H */
