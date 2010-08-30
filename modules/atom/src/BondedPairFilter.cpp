/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondedPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondedPairFilter
::BondedPairFilter(){
}

ParticlesTemp BondedPairFilter
::get_input_particles(const ParticlePair& t) const {
  ParticlesTemp ret;
  ret.push_back(t[0]);
  ret.push_back(t[1]);
  if (!Bonded::particle_is_instance(t[1])
      || ! Bonded::particle_is_instance(t[0])) {
  } else {
    Bonded ba(t[0]);
    Bonded bb(t[1]);
    Bond bd=get_bond(ba, bb);
    if (bd) ret.push_back(bd);
  }
  return ret;
}


ContainersTemp
BondedPairFilter::get_input_containers(const ParticlePair&) const {
  return ContainersTemp();
}
void BondedPairFilter::do_show(std::ostream &) const {
}

IMPATOM_END_NAMESPACE
