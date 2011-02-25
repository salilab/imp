/**
 *  \file atom/BerendsenThermostatOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H
#define IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H

#include "atom_config.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Implement the Berendsen thermostat.
/**  */
class IMPATOMEXPORT BerendsenThermostatOptimizerState : public OptimizerState
{
 public:
  BerendsenThermostatOptimizerState(const Particles &pis,
                                    double temperature,
                                    unsigned skip_steps);

  //! Set the number of update calls to skip between rescaling.
  void set_skip_steps(unsigned skip_steps) {
    skip_steps_ = skip_steps;
  }

  //! Set the particles to use.
  void set_particles(const Particles &pis) {
    pis_=pis;
  }

  IMP_OPTIMIZER_STATE(BerendsenThermostatOptimizerState);

private:
  void do_therm();
  Particles pis_;
  double temperature_;
  unsigned skip_steps_;
  unsigned call_number_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(BerendsenThermostatOptimizerState,
            BerendsenThermostatOptimizerStates);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H */
