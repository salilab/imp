/**
 *  \file FixedRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/FixedRefiner.h"
#include <IMP/log.h>


IMPCORE_BEGIN_NAMESPACE

FixedRefiner::FixedRefiner(const ParticlesTemp &ps):
  Refiner("FixedRefiner%d"), ps_(ps){
  IMP_LOG_VERBOSE( "Created fixed particle refiner with " << ps.size()
          << " particles" << std::endl);
}

void FixedRefiner::do_show(std::ostream &out) const {
  out << "producing " << ps_.size() << " particles"
      << std::endl;
}

bool FixedRefiner::get_can_refine(Particle *) const {
  return true;
}

Particle* FixedRefiner::get_refined(Particle *, unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  return ps_[i];
}

unsigned int FixedRefiner::get_number_of_refined(Particle *) const {
  IMP_CHECK_OBJECT(this);
  return ps_.size();
}

const ParticlesTemp FixedRefiner::get_refined(Particle *) const {
  return ps_;
}

ParticlesTemp FixedRefiner::get_input_particles(Particle *) const {
  return ParticlesTemp();
}

ContainersTemp FixedRefiner::get_input_containers(Particle *) const {
  return ContainersTemp();
}

IMPCORE_END_NAMESPACE
