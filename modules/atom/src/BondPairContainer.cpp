/**
 *  \file BondPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/BondPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/PairScore.h"

#define FOREACH(expr)\
  unsigned int sz= BondPairContainer::get_number_of_particle_pairs();   \
  for (unsigned int i=0; i< sz; ++i) {                                  \
    ParticlePair pp= BondPairContainer::get_particle_pair(i);           \
    Particle *a= pp[0];                                                 \
    Particle *b= pp[1];                                                 \
    expr;                                                               \
  }


IMPATOM_BEGIN_NAMESPACE

BondPairContainer
::BondPairContainer(SingletonContainer *sc, bool): sc_(sc){
}

BondPairContainer
::BondPairContainer(SingletonContainer *sc): sc_(sc){
  set_added_and_removed_containers(
       create_untracked_container(sc_->get_removed_singletons_container()),
       create_untracked_container(sc_->get_added_singletons_container()));
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

IMP_PAIR_CONTAINER_METHODS_FROM_FOREACH(BondPairContainer);

ObjectsTemp BondPairContainer::get_input_objects() const {
  ObjectsTemp ret(get_number_of_particle_pairs()+1);
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= sc_->get_particle(i);
  }
  ret.back()= sc_;
  return ret;
}

IMPATOM_END_NAMESPACE
