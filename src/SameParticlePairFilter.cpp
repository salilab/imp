/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/membrane/SameParticlePairFilter.h"
#include "IMP/core/rigid_bodies.h"

IMPMEMBRANE_BEGIN_NAMESPACE

SameParticlePairFilter
::SameParticlePairFilter(){
}

bool SameParticlePairFilter::get_contains(const ParticlePair &pp)
 const {
      return pp[0] == pp[1];
}

ParticlesTemp SameParticlePairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  return ret;
}

ContainersTemp
SameParticlePairFilter::get_input_containers(Particle*p) const {
  return ContainersTemp(1, p);
}

void SameParticlePairFilter::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
