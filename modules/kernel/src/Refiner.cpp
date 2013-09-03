/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Refiner.h"
#include "IMP/kernel/internal/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

Refiner::Refiner(std::string name) : Object(name) {}

ParticleIndexes Refiner::get_refined_indexes(kernel::Model *m, ParticleIndex pi) const {
  return get_indexes(get_refined(m->get_particle(pi)));
}

IMPKERNEL_END_NAMESPACE
