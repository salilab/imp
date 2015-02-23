/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/LangevinThermostatOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
#include <IMP/base/random.h>
IMPATOM_BEGIN_NAMESPACE

LangevinThermostatOptimizerState::LangevinThermostatOptimizerState(
    Model *m, ParticleIndexesAdaptor pis, Float temperature, double gamma)
    : OptimizerState(m, "LangevinThermostatOptimizerState%1%"),
      pis_(kernel::get_particles(m, pis)),
      temperature_(temperature),
      gamma_(gamma) {
  IMP_LOG_VERBOSE("Thermostat on " << pis_ << std::endl);
}

void LangevinThermostatOptimizerState::do_update(unsigned int) {
  rescale_velocities();
}

IMP_GCC_DISABLE_WARNING(-Wuninitialized)
void LangevinThermostatOptimizerState::rescale_velocities() const {
  static const double gas_constant = 8.31441e-7;
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  double c1 = exp(-gamma_ * md->get_last_time_step());
  double c2 = sqrt((1.0 - c1) * gas_constant * temperature_);
  IMP_INTERNAL_CHECK(md,
                     "Can only use velocity scaling with "
                     "the molecular dynamics optimizer.");
  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<base::RandomNumberGenerator &,
                           boost::normal_distribution<Float> >
      sampler(base::random_number_generator, mrng);
  for (unsigned int i = 0; i < pis_.size(); ++i) {
    kernel::Particle *p = pis_[i];
    double mass = Mass(p).get_mass();
    LinearVelocity lv(p);
    lv.set_velocity(c1 * lv.get_velocity()
                    + c2 * sqrt((c1 + 1.0) / mass)
                      * algebra::Vector3D(sampler(), sampler(), sampler()));
  }
}

IMPATOM_END_NAMESPACE
