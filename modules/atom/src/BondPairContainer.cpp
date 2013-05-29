/**
 *  \file BondPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/PairScore.h"

IMPATOM_BEGIN_NAMESPACE

BondPairContainer::BondPairContainer(SingletonContainer *sc)
    : PairContainer(sc->get_model(), "BondPairContainer%1%"), sc_(sc) {}

ParticleIndexPairs BondPairContainer::get_indexes() const {
  ParticleIndexes ia = sc_->get_indexes();
  ParticleIndexPairs ret;
  ret.reserve(ia.size());
  for (unsigned int i = 0; i < ia.size(); ++i) {
    Bond b(get_model(), ia[i]);
    ret.push_back(ParticleIndexPair(b.get_bonded(0).get_particle_index(),
                                    b.get_bonded(1).get_particle_index()));
  }
  return ret;
}

void BondPairContainer::do_show(std::ostream &out) const {
  out << "container " << *sc_ << std::endl;
}

ParticleIndexes BondPairContainer::get_all_possible_indexes() const {
  ParticleIndexes scapp = sc_->get_all_possible_indexes();
  ParticleIndexes ret(3 * scapp.size());
  for (unsigned int i = 0; i < scapp.size(); ++i) {
    ret[i * 3] = scapp[i];
    ret[i * 3 + 1] =
        Bond(get_model(), scapp[i]).get_bonded(0).get_particle_index();
    ret[i * 3 + 2] =
        Bond(get_model(), scapp[i]).get_bonded(1).get_particle_index();
  }
  return ret;
}

ParticleIndexPairs BondPairContainer::get_range_indexes() const {
  ParticleIndexes ia = sc_->get_range_indexes();
  ParticleIndexPairs ret;
  ret.reserve(ia.size());
  for (unsigned int i = 0; i < ia.size(); ++i) {
    Bond b(get_model(), ia[i]);
    ret.push_back(ParticleIndexPair(b.get_bonded(0).get_particle_index(),
                                    b.get_bonded(1).get_particle_index()));
  }
  return ret;
}

void BondPairContainer::do_before_evaluate() {
  set_is_changed(sc_->get_is_changed());
}

IMPATOM_END_NAMESPACE
