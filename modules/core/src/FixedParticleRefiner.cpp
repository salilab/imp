/**
 *  \file FixedParticleRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/FixedParticleRefiner.h"
#include <IMP/log.h>


IMPCORE_BEGIN_NAMESPACE

FixedParticleRefiner::FixedParticleRefiner(const Particles &ps): ps_(ps){
  IMP_LOG(VERBOSE, "Created fixed particle refiner with " << ps.size()
          << " particles" << std::endl);
}

FixedParticleRefiner::~FixedParticleRefiner(){}

void FixedParticleRefiner::show(std::ostream &out) const {
  out << "FixedParticleRefiner on " << ps_.size() << " particles"
      << std::endl;
}

bool FixedParticleRefiner::get_can_refine(Particle *) const {
  return true;
}

Particles FixedParticleRefiner::get_refined(Particle *) const {
  IMP_CHECK_OBJECT(this);
  return ps_;
}

void FixedParticleRefiner::cleanup_refined(Particle *,
                                           Particles &,
                                   DerivativeAccumulator *) const {
}

IMPCORE_END_NAMESPACE
