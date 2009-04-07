/**
 *  \file FixedRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/FixedRefiner.h"
#include <IMP/log.h>


IMPCORE_BEGIN_NAMESPACE

IMP_LIST_IMPL(FixedRefiner, Particle, particle, Particle*,,,)

FixedRefiner::FixedRefiner(const Particles &ps){
  IMP_LOG(VERBOSE, "Created fixed particle refiner with " << ps.size()
          << " particles" << std::endl);
  set_particles(ps);
}

void FixedRefiner::show(std::ostream &out) const {
  out << "FixedRefiner on " << get_number_of_particles() << " particles"
      << std::endl;
}

bool FixedRefiner::get_can_refine(Particle *) const {
  return true;
}

Particle* FixedRefiner::get_refined(Particle *, unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  return get_particle(i);
}

unsigned int FixedRefiner::get_number_of_refined(Particle *) const {
  IMP_CHECK_OBJECT(this);
  return get_number_of_particles();
}

const Particles FixedRefiner::get_refined(Particle *p) const {
  return Particles(particles_begin(), particles_end());
}

IMPCORE_END_NAMESPACE
