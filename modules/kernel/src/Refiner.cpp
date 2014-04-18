/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Refiner.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/Model.h"

IMPKERNEL_BEGIN_NAMESPACE

Refiner::Refiner(std::string name) : Object(name) {}

ParticleIndexes Refiner::get_refined_indexes(kernel::Model *m,
                                             ParticleIndex pi) const {
  ParticlesTemp ps = get_refined( m->get_particle(pi) );
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for(unsigned int i = 0; i < ps.size(); i++) {
      if (ps[i]->get_model() != m) {
        IMP_THROW("Refined particles model does not match parent model - "
                  "this is critical if get_refined_indexes() is used.",
                  IMP::base::ValueException);
      }
    }
  }
  return get_indexes(ps);
}

IMPKERNEL_END_NAMESPACE
