/**
 *  \file Mover.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/MonteCarloMover.h"
#include "IMP/core/MoverBase.h"

IMPCORE_BEGIN_NAMESPACE

MonteCarloMover::MonteCarloMover(Model *m, std::string name)
    : ModelObject(m, name), has_move_(false) {
  reset_statistics();
}


///////////////////////////// DEPRECATED_STUFF

MoverBase::MoverBase(const kernel::ParticlesTemp &ps, const FloatKeys &keys,
                     std::string name)
    : Mover(IMP::internal::get_model(ps), name),
      keys_(keys),
      particles_(IMP::internal::get_index(ps)) {
  IMPCORE_DEPRECATED_OBJECT_DEF(2.1, "Use MonteCarloMover instead");
}

ParticlesTemp MoverBase::propose_move(Float f) {
  values_.resize(particles_.size(), Floats(keys_.size(), 0));
  for (unsigned int i = 0; i < particles_.size(); ++i) {
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      values_[i][j] = get_value(i, j);
    }
  }
  do_move(f);
  return IMP::internal::get_particle(get_model(), particles_);
}

void MoverBase::reset_move() {
  for (unsigned int i = 0; i < particles_.size(); ++i) {
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      get_model()->set_attribute(keys_[j], particles_[i], values_[i][j]);
    }
  }
}

void MoverBase::do_propose_value(unsigned int i, unsigned int j, Float t) {
  IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
  IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
  if (get_model()->get_is_optimized(keys_[j], particles_[i])) {
    get_model()->set_attribute(keys_[j], particles_[i], t);
    IMP_USAGE_CHECK_FLOAT_EQUAL(
                        get_model()->get_attribute(keys_[j], particles_[i]), t,
                                "Tried to set, but it didn't work.");
  } else {
      IMP_LOG_TERSE("Dropping change to unoptimized attribute: "
                    << keys_[j] << " of particle "
                    << get_model()->get_particle(particles_[i])->get_name()
                    << std::endl);
  }
}

ParticlesTemp MoverBase::get_output_particles() const {
  return IMP::internal::get_particle(get_model(), particles_);
}

std::string MoverBase::get_particle_name(unsigned int i) const {
  return get_model()->get_particle(particles_[i])->get_name();
}

Float MoverBase::get_value(unsigned int i, unsigned int j) const {
  IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
  IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
  return get_model()->get_attribute(keys_[j], particles_[i]);
}

void MoverBase::propose_value(unsigned int i, unsigned int j, Float t) {
  do_propose_value(i, j, t);
}

IMPCORE_END_NAMESPACE
