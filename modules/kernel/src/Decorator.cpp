/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Decorator.h"
#include "IMP/Particle.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
extern Vector<std::pair<ParticleFunction, ParticleFunction> >
    particle_validators;

void add_particle_check(ParticleFunction instance, ParticleFunction check) {
  particle_validators.push_back(std::make_pair(instance, check));
}

IMPKERNEL_END_INTERNAL_NAMESPACE

IMPKERNEL_BEGIN_NAMESPACE

Decorator::Decorator(ParticleAdaptor p)
  : model_(p.get_model()), pi_(p.get_particle_index()), is_valid_(true) {}

uint32_t Decorator::get_model_id() const {
  return model_->get_unique_id();
}

void Decorator::set_model_from_id(uint32_t model_id) {
  Model *m = Model::get_by_unique_id(model_id);
  if (!m) {
    IMP_THROW("Cannot unserialize Decorator as it refers to a "
              "Model that does not exist", ValueException);
  } else {
    model_ = m;
  }
}

void check_particle(Model *m, ParticleIndex pi) {
  for (unsigned int i = 0; i < internal::particle_validators.size(); ++i) {
    if (internal::particle_validators[i].first(m, pi)) {
      internal::particle_validators[i].second(m, pi);
    }
  }
}

Undecorator::Undecorator(Model *m, std::string name)
    : Object(name), m_(m) {}

IMPKERNEL_END_NAMESPACE
