/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/BondedPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondedPairFilter
::BondedPairFilter(){
}

bool BondedPairFilter
::get_contains_particle_pair(ParticlePair pp) const {
  if (!Bonded::particle_is_instance(pp.first)
      || ! Bonded::particle_is_instance(pp.second)) {
    return false;
  }

  Bonded ba(pp.first);
  Bonded bb(pp.second);
  Bond bd=get_bond(ba, bb);
  return bd != Bond();
}

ParticlesTemp BondedPairFilter
::get_input_particles(ParticlePair t) const {
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


ObjectsTemp
BondedPairFilter::get_input_objects(ParticlePair pt) const {
  return ObjectsTemp();
}
void BondedPairFilter::show(std::ostream &out) const {
  out << "BondPairFilter" << std::endl;
}

IMPATOM_END_NAMESPACE
