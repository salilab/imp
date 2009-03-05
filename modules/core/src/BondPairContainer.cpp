/**
 *  \file BondDecoratorPairContainer.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/BondPairContainer.h"

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

BondPairContainer
::BondPairContainer(){
}

bool BondPairContainer
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

unsigned int BondPairContainer
::get_number_of_particle_pairs() const {
  return 0;
}

ParticlePair BondPairContainer
::get_particle_pair(unsigned int i) const {
  throw InvalidStateException("BondPairContainer does" \
                              " not contain any pairs");
  return ParticlePair();
}


void BondPairContainer::show(std::ostream &out) const {
  out << "BondPairContainer" << std::endl;
}

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
