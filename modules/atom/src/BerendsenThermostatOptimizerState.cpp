/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BerendsenThermostatOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
IMPATOM_BEGIN_NAMESPACE

BerendsenThermostatOptimizerState::BerendsenThermostatOptimizerState(
    const Particles &pis, Float temperature, unsigned skip_steps) :
    pis_(pis), temperature_(temperature), skip_steps_(skip_steps),
    call_number_(0)
{
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void BerendsenThermostatOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    do_therm();
  }
  ++call_number_;
}

void BerendsenThermostatOptimizerState::do_therm()
{
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  IMP_INTERNAL_CHECK(md, "Can only use velocity scaling with "
             "the molecular dynamics optimizer.");
  double rescale=sqrt(1.0-(md->get_time_step()/temperature_)
                      *(1.0-(temperature_/
                md->get_kinetic_temperature(md->get_kinetic_energy()))));
  for (unsigned int i=0; i< pis_.size(); ++i) {
    Particle *p = pis_[i];
    for (int i = 0; i < 3; ++i) {
      double velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
   }
 }
}

void BerendsenThermostatOptimizerState::do_show(std::ostream &out) const
{
  out << "Berendsen thermostate with " << temperature_ << " every "
      << skip_steps_ << " steps" << std::endl;
}

IMPATOM_END_NAMESPACE
