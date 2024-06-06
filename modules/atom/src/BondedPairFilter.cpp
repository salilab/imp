/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondedPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondedPairFilter::BondedPairFilter() : PairPredicate("BondedPairFilter%1%") {}

int BondedPairFilter::get_value_index(
    Model *m, const ParticleIndexPair &pip) const {
  if (!Bonded::get_is_setup(m, std::get<0>(pip))
      || !Bonded::get_is_setup(m, std::get<1>(pip))) {
    return false;
  }
  Bonded ba(m, std::get<0>(pip));
  Bonded bb(m, std::get<1>(pip));
  Bond bd = get_bond(ba, bb);
  return bd != Bond();
}

ModelObjectsTemp BondedPairFilter::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (Bonded::get_is_setup(m, pis[i])) {
      Bonded b(m, pis[i]);
      for (unsigned int i = 0; i < b.get_number_of_bonds(); ++i) {
        ret.push_back(b.get_bond(i));
      }
    }
  }
  return ret;
}

IMPATOM_END_NAMESPACE
