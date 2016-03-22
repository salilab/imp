/**
 *  \file VelocityScalingOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/VelocityScalingOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>

IMPATOM_BEGIN_NAMESPACE

VelocityScalingOptimizerState::VelocityScalingOptimizerState(
    Model *m, ParticleIndexesAdaptor pis, double temp)
    : OptimizerState(m, "VelocityScalingOptimizerState%1%"),
      temperature_(temp) {
  IMP_FOREACH(ParticleIndex pi, pis) {
    pis_.push_back(m->get_particle(pi));
  }
}

void VelocityScalingOptimizerState::do_update(unsigned int) {
  rescale_velocities();
}

void VelocityScalingOptimizerState::rescale_velocities() const {
  MolecularDynamics *md = dynamic_cast<MolecularDynamics *>(get_optimizer());
  IMP_INTERNAL_CHECK(md,
                     "Can only use velocity scaling with "
                     "the molecular dynamics optimizer.");

  Float ekinetic = md->get_kinetic_energy();
  Float tkinetic = md->get_kinetic_temperature(ekinetic);
  if (tkinetic > 1e-8) {
    Float scale = std::sqrt(temperature_ / tkinetic);
    for (unsigned i = 0; i < pis_.size(); ++i) {
      LinearVelocity v(pis_[i]);
      v.set_velocity(v.get_velocity() * scale);
    }
  }
}

IMPATOM_END_NAMESPACE
