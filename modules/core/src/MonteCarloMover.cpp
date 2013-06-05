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
    : ModelObject(m, name) {
  reset_statistics();
}


MoverBase::MoverBase(const ParticlesTemp &ps, const FloatKeys &keys,
                     std::string name)
      : Mover(IMP::internal::get_model(ps), name),
        keys_(keys),
        particles_(IMP::internal::get_index(ps)) {
    IMPCORE_DEPRECATED_CLASS_DEF(2.1, "Use MonteCarloMover instead");
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

IMPCORE_END_NAMESPACE
