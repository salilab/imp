/**
 *  \file FixedParticleRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/FixedParticleRefiner.h"
#include <IMP/log.h>


IMPCORE_BEGIN_NAMESPACE

IMP_LIST_IMPL(FixedParticleRefiner, Particle, particle, Particle*,,,)

FixedParticleRefiner::FixedParticleRefiner(const Particles &ps){
  IMP_LOG(VERBOSE, "Created fixed particle refiner with " << ps.size()
          << " particles" << std::endl);
  set_particles(ps);
}

FixedParticleRefiner::~FixedParticleRefiner(){}

void FixedParticleRefiner::show(std::ostream &out) const {
  out << "FixedParticleRefiner on " << get_number_of_particles() << " particles"
      << std::endl;
}

bool FixedParticleRefiner::get_can_refine(Particle *) const {
  return true;
}

Particles FixedParticleRefiner::get_refined(Particle *) const {
  IMP_CHECK_OBJECT(this);
  return Particles(particles_begin(), particles_end());
}

void FixedParticleRefiner::cleanup_refined(Particle *,
                                           Particles &,
                                   DerivativeAccumulator *) const {
}

IMPCORE_END_NAMESPACE
