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

unsigned int BondPairFilter
::get_number_of_particle_pairs() const {
  return 0;
}

ParticlePair BondPairFilter
::get_particle_pair(unsigned int i) const {
  throw InvalidStateException("BondPairFilter does" \
                              " not contain any pairs");
  return ParticlePair();
}


void BondPairFilter::show(std::ostream &out) const {
  out << "BondPairFilter" << std::endl;
}



void BondPairFilter
 ::apply(const PairModifier *sm) {
   IMP_failure("Not a real container", ErrorException);
 }

void BondPairFilter
::apply(const PairModifier *sm, DerivativeAccumulator &da) {
   IMP_failure("Not a real container", ErrorException);
}

 double BondPairFilter
::evaluate(const PairScore *s,
           DerivativeAccumulator *da) const {
   IMP_failure("Not a real container", ErrorException);
   return 0;
 }

ParticlePairsTemp BondPairFilter::get_particle_pairs() const {
  IMP_failure("Not a real container", ErrorException);
  return ParticlePairsTemp();
}


unsigned int BondPairFilter::get_revision() const {
  return 0;
}
IMPATOM_END_NAMESPACE
