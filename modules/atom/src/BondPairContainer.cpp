/**
 *  \file BondPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/PairScore.h"



IMPATOM_BEGIN_NAMESPACE

BondPairContainer
::BondPairContainer(SingletonContainer *sc, bool): sc_(sc){
}

BondPairContainer
::BondPairContainer(SingletonContainer *sc): sc_(sc){
}


bool
BondPairContainer::get_contents_changed() const {
  return sc_->get_contents_changed();
}


bool BondPairContainer
::get_contains_particle_pair(const ParticlePair& pp) const {
  if (!Bonded::particle_is_instance(pp[0])
      || ! Bonded::particle_is_instance(pp[1])) {
    return false;
  }

  Bonded ba(pp[0]);
  Bonded bb(pp[1]);
  Bond bd=get_bond(ba, bb);
  return sc_->get_contains_particle(bd);
}


ParticleIndexPairs
BondPairContainer::get_indexes() const {
  ParticleIndexes ia= sc_->get_indexes();
  ParticleIndexPairs ret; ret.reserve(ia.size());
  for (unsigned int i=0; i< ia.size(); ++i) {
    Bond b(get_model(), ia[i]);
    ret.push_back(ParticleIndexPair(b.get_bonded(0).get_particle_index(),
                                    b.get_bonded(1).get_particle_index()));
  }
  return ret;
}

void BondPairContainer::do_show(std::ostream &out) const {
  out << "container " << *sc_ << std::endl;
}


ParticlesTemp BondPairContainer::get_contained_particles() const {
  ParticlesTemp ret(3*sc_->get_number_of_particles());
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    ret[i*3]= sc_->get_particle(i);
    ret[i*3+1]= Bond(sc_->get_particle(i)).get_bonded(0);
    ret[i*3+2]= Bond(sc_->get_particle(i)).get_bonded(1);
  }
  return ret;
}

ParticleIndexPairs BondPairContainer::get_all_possible_indexes() const {
  ParticleIndexes ia= sc_->get_all_possible_indexes();
  ParticleIndexPairs ret; ret.reserve(ia.size());
  for (unsigned int i=0; i< ia.size(); ++i) {
    Bond b(get_model(), ia[i]);
    ret.push_back(ParticleIndexPair(b.get_bonded(0).get_particle_index(),
                                    b.get_bonded(1).get_particle_index()));
  }
  return ret;
}


IMPATOM_END_NAMESPACE
