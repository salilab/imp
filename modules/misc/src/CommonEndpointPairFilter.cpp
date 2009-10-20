/**
 *  \file CommonEndpointPairFilter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/misc/CommonEndpointPairFilter.h"
#include <IMP/atom/bond_decorators.h>


IMPMISC_BEGIN_NAMESPACE

CommonEndpointPairFilter::CommonEndpointPairFilter( ){
}


void CommonEndpointPairFilter::show(std::ostream &out) const {
  out << "CommonEndpointPairFilter" << std::endl;
}

bool CommonEndpointPairFilter
::get_contains_particle_pair(ParticlePair p) const {
  if (!IMP::atom::Bond::particle_is_instance(p[0])
      || !IMP::atom::Bond::particle_is_instance(p[1])) {
    return false;
  } else {
    IMP::atom::Bond b0(p[0]);
    IMP::atom::Bond b1(p[1]);
    return b0.get_bonded(0) == b1.get_bonded(1)
      || b0.get_bonded(1) == b1.get_bonded(0)
      || b0.get_bonded(0) == b1.get_bonded(0)
      || b0.get_bonded(1) == b1.get_bonded(1);
  }
}


ParticlesTemp
CommonEndpointPairFilter::get_input_particles(ParticlePair pt)
  const {
  ParticlesTemp ret;
  ret.reserve(3);
  ret.push_back(pt[0]);
  ret.push_back(pt[1]);
  if (IMP::atom::Bond::particle_is_instance(pt[0])){
    IMP::atom::Bond b(pt[0]);
    ret.push_back(b.get_bonded(0));
    ret.push_back(b.get_bonded(1));
  }
  if (IMP::atom::Bond::particle_is_instance(pt[1])){
    IMP::atom::Bond b(pt[1]);
    ret.push_back(b.get_bonded(0));
    ret.push_back(b.get_bonded(1));
  }
  return ret;
}

ObjectsTemp
CommonEndpointPairFilter::get_input_objects(ParticlePair pt) const {
  return ObjectsTemp();
}

IMPMISC_END_NAMESPACE
