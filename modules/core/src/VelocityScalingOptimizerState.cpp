/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/VelocityScalingOptimizerState.h>
#include <IMP/core/MolecularDynamics.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

VelocityScalingOptimizerState::VelocityScalingOptimizerState(
    const Particles &pis, Float temperature, unsigned skip_steps) :
    pis_(pis), temperature_(temperature), skip_steps_(skip_steps),
    call_number_(0)
{
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void VelocityScalingOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    rescale_velocities();
  }
  ++call_number_;
}

void VelocityScalingOptimizerState::rescale_velocities() const
{
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  IMP_assert(md, "Can only use velocity scaling with "
             "the molecular dynamics optimizer.");

  Float ekinetic = md->get_kinetic_energy();
  Float tkinetic = md->get_kinetic_temperature(ekinetic);
  if (tkinetic > 1e-8) {
    Float scale = std::sqrt(temperature_ / tkinetic);
    for (unsigned i = 0; i < pis_.size(); ++i) {
      for (unsigned j = 0; j < 3; ++j) {
        Float v = pis_[i]->get_value(vs_[j]) * scale;
        pis_[i]->set_value(vs_[j], v);
      }
    }
  }
}

void VelocityScalingOptimizerState::show(std::ostream &out) const
{
  out << "Rescaling velocities to " << temperature_ << " every "
      << skip_steps_ << " steps" << std::endl;
}

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
