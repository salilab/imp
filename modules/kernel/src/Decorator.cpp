/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/kernel/Decorator.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
extern base::Vector<std::pair<ParticleFunction, ParticleFunction> >
particle_validators;

void add_particle_check(ParticleFunction instance, ParticleFunction check) {
  particle_validators.push_back(std::make_pair(instance, check));
}

IMPKERNEL_END_INTERNAL_NAMESPACE

IMPKERNEL_BEGIN_NAMESPACE

void check_particle(Particle*p) {
  for (unsigned int i=0; i < internal::particle_validators.size(); ++i) {
    if (internal::particle_validators[i].first(p)) {
      internal::particle_validators[i].second(p);
    }
  }
}

Undecorator::Undecorator(Model *m, std::string name): base::Object(name),
                                                      m_(m) {}

IMPKERNEL_END_NAMESPACE
