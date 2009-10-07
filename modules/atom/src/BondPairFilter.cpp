/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/BondPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondPairFilter
::BondPairFilter(){
}

bool BondPairFilter
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

ParticlesTemp BondPairFilter
::get_used_particles(const ParticlePairsTemp &t) const {
  ParticlesTemp ret;
  ret.reserve(t.size()*3);
  for (unsigned int i=0; i< t.size(); ++i) {
    ret.push_back(t[i][0]);
    ret.push_back(t[i][1]);
    if (!Bonded::particle_is_instance(t[i][1])
        || ! Bonded::particle_is_instance(t[i][0])) {
      continue;
    }

    Bonded ba(t[i][0]);
    Bonded bb(t[i][1]);
    Bond bd=get_bond(ba, bb);
    if (bd) ret.push_back(bd);
  }
  return ret;
}

void BondPairFilter::show(std::ostream &out) const {
  out << "BondPairFilter" << std::endl;
}

IMPATOM_END_NAMESPACE
