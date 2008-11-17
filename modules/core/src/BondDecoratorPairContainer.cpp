/**
 *  \file BondDecoratorPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/BondDecoratorPairContainer.h"

IMPCORE_BEGIN_NAMESPACE

BondDecoratorPairContainer
::BondDecoratorPairContainer(){
}

BondDecoratorPairContainer::~BondDecoratorPairContainer(){}

bool BondDecoratorPairContainer
::get_contains_particle_pair(ParticlePair pp) const {
  if (!BondedDecorator::is_instance_of(pp.first)
      || ! BondedDecorator::is_instance_of(pp.second)) {
    return false;
  }

  BondedDecorator ba(pp.first);
  BondedDecorator bb(pp.second);
  BondDecorator bd=get_bond(ba, bb);
  return bd != BondDecorator();
}

unsigned int BondDecoratorPairContainer
::get_number_of_particle_pairs() const {
  return 0;
}

ParticlePair BondDecoratorPairContainer
::get_particle_pair(unsigned int i) const {
  throw InvalidStateException("BondDecoratorPairContainer does" \
                              " not contain any pairs");
  return ParticlePair();
}


void BondDecoratorPairContainer::show(std::ostream &out) const {
  out << "BondDecoratorPairContainer" << std::endl;
}

IMPCORE_END_NAMESPACE
