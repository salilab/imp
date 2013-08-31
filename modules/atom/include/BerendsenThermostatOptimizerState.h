/**
 *  \file IMP/atom/BerendsenThermostatOptimizerState.h
 *  \brief Maintains temperature during molecular dynamics using a
 *         Berendsen thermostat.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H
#define IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Maintains temperature during molecular dynamics.
/** The thermostat scales velocities using the algorithm described
    in H. J. C. Berendsen, J. P. M. Postma,
    W. F. van Gunsteren, A. DiNola, and J. R. Haak "Molecular dynamics
    with coupling to an external bath", Journal of Chemical Physics 81
    pp. 3684-3690 (1984).

    At each update, velocities are rescaled by \f[
    \lambda = \left[1 + \frac{\Delta t}{\tau_T}
                        \left( \frac{T}{T_k} -1\right)\right]^{1/2}
    \f]
    where \f$\Delta t\f$ is the molecular dynamics timestep, \f$\tau_T\f$
    is the coupling constant (in fs) of the thermostat, \f$T\f$ is the
    thermostat temperature, and \f$T_k\f$ is the instantaneous (kinetic)
    temperature of the dynamics. (This is equation 11 from the reference above.)
 */
class IMPATOMEXPORT BerendsenThermostatOptimizerState : public OptimizerState {
 public:
  BerendsenThermostatOptimizerState(const kernel::Particles &pis, double temperature,
                                    double tau);

  //! Set the particles to use.
  void set_particles(const kernel::Particles &pis) { pis_ = pis; }

  double get_temperature() { return temperature_; }

  double get_tau() { return tau_; }

  void set_temperature(double temperature) { temperature_ = temperature; }

  void set_tau(double tau) { tau_ = tau; }

  //! Rescale the velocities now
  void rescale_velocities() const;

  IMP_OBJECT_METHODS(BerendsenThermostatOptimizerState);

 protected:
    virtual void do_update(unsigned int) IMP_OVERRIDE;
 private:
  kernel::Particles pis_;
  double temperature_;
  double tau_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];
};

IMP_OBJECTS(BerendsenThermostatOptimizerState,
            BerendsenThermostatOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BERENDSEN_THERMOSTAT_OPTIMIZER_STATE_H */
