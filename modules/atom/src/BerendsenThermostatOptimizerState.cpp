/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics using a
 *         Berendsen thermostat.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BerendsenThermostatOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>

IMPATOM_BEGIN_NAMESPACE

BerendsenThermostatOptimizerState::BerendsenThermostatOptimizerState(
    const Particles &pis, double temperature, double tau)
    : OptimizerState(pis[0]->get_model(),
                     "BerendsenThermostatOptimizerState%1%"),
      pis_(pis),
      temperature_(temperature),
      tau_(tau) {
}

void BerendsenThermostatOptimizerState::do_update(unsigned int) {
  rescale_velocities();
}

void BerendsenThermostatOptimizerState::rescale_velocities() const {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Rescaling velocities" << std::endl);
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  IMP_INTERNAL_CHECK(md,
                     "Can only use velocity scaling with "
                     "the molecular dynamics optimizer.");

  double kinetic_temp = md->get_kinetic_temperature(md->get_kinetic_energy());
  double rescale = std::sqrt(1.0 + (md->get_last_time_step() / tau_) *
                                       (temperature_ / kinetic_temp - 1.0));

  for (unsigned int i = 0; i < pis_.size(); ++i) {
    Particle *p = pis_[i];
    LinearVelocity v(p);
    v.set_velocity(v.get_velocity() * rescale);
  }
}

IMPATOM_END_NAMESPACE
