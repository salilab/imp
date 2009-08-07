/**
 *  \file BondPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/BondPairContainer.h"

IMPATOM_BEGIN_NAMESPACE

BondPairContainer
::BondPairContainer(SingletonContainer *sc): sc_(sc){
}

bool BondPairContainer
::get_contains_particle_pair(ParticlePair pp) const {
  if (!Bonded::particle_is_instance(pp.first)
      || ! Bonded::particle_is_instance(pp.second)) {
    return false;
  }

  Bonded ba(pp.first);
  Bonded bb(pp.second);
  Bond bd=get_bond(ba, bb);
  return sc_->get_contains_particle(bd);
}

unsigned int BondPairContainer
::get_number_of_particle_pairs() const {
  return sc_->get_number_of_particles();
}

ParticlePair BondPairContainer
::get_particle_pair(unsigned int i) const {
  Bond bd(sc_->get_particle(i));
  return ParticlePair(bd.get_bonded(0), bd.get_bonded(1));
}


void BondPairContainer::show(std::ostream &out) const {
  out << "BondPairContainer" << std::endl;
}


void BondPairContainer
::apply(const PairModifier *sm) {
  unsigned int sz= BondPairContainer::get_number_of_particle_pairs();
  for (unsigned int i=0; i< sz; ++i) {
     ParticlePair pp= BondPairContainer::get_particle_pair(i);
     sm->apply(pp[0], pp[1]);
   }
}

void BondPairContainer
::apply(const PairModifier *sm, DerivativeAccumulator *da) {
  unsigned int sz= BondPairContainer::get_number_of_particle_pairs();
  for (unsigned int i=0; i< sz; ++i) {
     ParticlePair pp= BondPairContainer::get_particle_pair(i);
     sm->apply(pp[0], pp[1], *da);
   }
}

 double BondPairContainer
::evaluate(const PairScore *s,
           DerivativeAccumulator *da) const {
   unsigned int sz= BondPairContainer::get_number_of_particle_pairs();
   double score=0;
   for (unsigned int i=0; i< sz; ++i) {
     ParticlePair pp= BondPairContainer::get_particle_pair(i);
     score+=s->evaluate(pp[0], pp[1], da);
   }
   return score;
 }

ParticlePairs BondPairContainer::get_particle_pairs() const {
  ParticlePairs ret(BondPairContainer::get_number_of_particle_pairs());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= BondPairContainer::get_particle_pair(i);
  }
  return ret;
}

unsigned int BondPairContainer::get_revision() const {
  return sc_->get_revision();
}


IMPATOM_END_NAMESPACE
