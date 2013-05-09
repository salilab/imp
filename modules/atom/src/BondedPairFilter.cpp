/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondedPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondedPairFilter::BondedPairFilter() : PairPredicate("BondedPairFilter%1%") {}

int BondedPairFilter::get_value(const ParticlePair &pp) const {
  if (!Bonded::particle_is_instance(pp[0]) ||
      !Bonded::particle_is_instance(pp[1])) {
    return false;
  }
  Bonded ba(pp[0]);
  Bonded bb(pp[1]);
  Bond bd = get_bond(ba, bb);
  return bd != Bond();
}

ParticlesTemp BondedPairFilter::get_input_particles(Particle *t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  if (Bonded::particle_is_instance(t)) {
    Bonded b(t);
    for (unsigned int i = 0; i < b.get_number_of_bonds(); ++i) {
      ret.push_back(b.get_bond(i));
    }
  }
  return ret;
}

ContainersTemp BondedPairFilter::get_input_containers(Particle *) const {
  return ContainersTemp();
}
void BondedPairFilter::do_show(std::ostream &) const {}

IMPATOM_END_NAMESPACE
