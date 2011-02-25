/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/LangevinThermostatOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
IMPATOM_BEGIN_NAMESPACE

LangevinThermostatOptimizerState
::LangevinThermostatOptimizerState(
                                   const Particles &pis,
                                   Float temperature, double tf,
                                   unsigned skip_steps) :
  pis_(pis), temperature_(temperature), time_friction_(tf),
  skip_steps_(skip_steps),
  call_number_(0)
{
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void LangevinThermostatOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    do_therm();
  }
  ++call_number_;
}

void LangevinThermostatOptimizerState::do_therm()
{
  static const double gas_constant = 8.31441e-7;
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  double c1 = exp(-time_friction_*md->get_time_step());
  double c2 = sqrt((1.0-c1)*gas_constant*temperature_);
  IMP_INTERNAL_CHECK(md, "Can only use velocity scaling with "
             "the molecular dynamics optimizer.");
  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
    sampler(random_number_generator, mrng);
  for (unsigned int i=0; i< pis_.size(); ++i) {
    Particle *p = pis_[i];
    double mass = Mass(p).get_mass();
    for (int i = 0; i < 3; ++i) {
      double velocity = p->get_value(vs_[i]);
      velocity = c1*velocity+c2*sqrt((c1+1.0)/mass)*sampler();
     p->set_value(vs_[i], velocity);
   }
 }
}

void LangevinThermostatOptimizerState::do_show(std::ostream &out) const
{
  out << "Berendsen thermostate with " << temperature_ << " every "
      << skip_steps_ << " steps" << std::endl;
}

IMPATOM_END_NAMESPACE
