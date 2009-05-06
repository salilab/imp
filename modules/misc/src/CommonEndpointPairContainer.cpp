/**
 *  \file CommonEndpointPairContainer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/misc/CommonEndpointPairContainer.h"
#include <IMP/atom/bond_decorators.h>


IMPMISC_BEGIN_NAMESPACE

CommonEndpointPairContainer::CommonEndpointPairContainer( ){
}


void CommonEndpointPairContainer::show(std::ostream &out) const {
  out << "CommonEndpointPairContainer" << std::endl;
}

bool CommonEndpointPairContainer
::get_contains_particle_pair(ParticlePair p) const {
  if (!IMP::atom::BondDecorator::is_instance_of(p[0])
      || !IMP::atom::BondDecorator::is_instance_of(p[1])) {
    return false;
  } else {
    IMP::atom::BondDecorator b0(p[0]);
    IMP::atom::BondDecorator b1(p[1]);
    return b0.get_bonded(0) == b1.get_bonded(1)
      || b0.get_bonded(1) == b1.get_bonded(0)
      || b0.get_bonded(0) == b1.get_bonded(0)
      || b0.get_bonded(1) == b1.get_bonded(1);
  }
}

unsigned int CommonEndpointPairContainer
::get_number_of_particle_pairs() const{
  return 0;
}

ParticlePair CommonEndpointPairContainer
::get_particle_pair(unsigned int i) const {
  IMP_failure("There are no pairs", ErrorException);
  return ParticlePair();
}


IMPMISC_END_NAMESPACE
