/**
 *  \file LangevinThermostatOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/LangevinThermostatOptimizerState.h>
#include <IMP/membrane/MolecularDynamicsWithWte.h>
#include <IMP/atom/Mass.h>
#include <IMP/base/random.h>
IMPMEMBRANE_BEGIN_NAMESPACE

LangevinThermostatOptimizerState
::LangevinThermostatOptimizerState(const ParticlesTemp &pis,
                         Float temperature, double gamma):
  OptimizerState(pis[0]->get_model(),"LTOptimizer")
{
  pis_ = pis;
  temperature_=temperature;
  gamma_=gamma;
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void LangevinThermostatOptimizerState::update()
{
    rescale_velocities();
}

//IMP_GCC_DISABLE_WARNING("-Wuninitialized")
void LangevinThermostatOptimizerState::rescale_velocities() const
{
  static const double gas_constant = 8.31441e-7;
  MolecularDynamicsWithWte *md =
   dynamic_cast<MolecularDynamicsWithWte *>(get_optimizer());
  double c1 = exp(-gamma_*md->get_last_time_step());
  double c2 = sqrt((1.0-c1)*gas_constant*temperature_);
  IMP_INTERNAL_CHECK(md, "Can only use velocity scaling with "
             "the molecular dynamics optimizer.");
  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<base::RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
    sampler(base::random_number_generator, mrng);
  for (unsigned int i=0; i< pis_.size(); ++i) {
    Particle *p = pis_[i];
    double mass = atom::Mass(p).get_mass();
    for (int i = 0; i < 3; ++i) {
      double velocity = p->get_value(vs_[i]);
      velocity = c1*velocity+c2*sqrt((c1+1.0)/mass)*sampler();
     p->set_value(vs_[i], velocity);
   }
 }
}

void LangevinThermostatOptimizerState::show(std::ostream &out) const
{
  out << "Langevin thermostat with " << temperature_ << std::endl;
}

IMPMEMBRANE_END_NAMESPACE
