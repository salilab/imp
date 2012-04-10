/**
 *  \file CommonEndpointPairFilter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/misc/CommonEndpointPairFilter.h"
#include <IMP/atom/bond_decorators.h>


IMPMISC_BEGIN_NAMESPACE

CommonEndpointPairFilter::CommonEndpointPairFilter( ){
}


void CommonEndpointPairFilter::do_show(std::ostream &) const {
}

bool CommonEndpointPairFilter
::get_contains(Model *m,  ParticleIndexPair p) const {
  Particle *pa= m->get_particle(p[0]);
  Particle *pb= m->get_particle(p[1]);
  if (!IMP::atom::Bond::particle_is_instance(pa)
      || !IMP::atom::Bond::particle_is_instance(pb)) {
    return false;
  } else {
    IMP::atom::Bond b0(m, p[0]);
    IMP::atom::Bond b1(m, p[1]);
    return b0.get_bonded(0) == b1.get_bonded(1)
      || b0.get_bonded(1) == b1.get_bonded(0)
      || b0.get_bonded(0) == b1.get_bonded(0)
      || b0.get_bonded(1) == b1.get_bonded(1);
  }
}


ParticlesTemp
CommonEndpointPairFilter::get_input_particles(Particle*p)
  const {
  ParticlesTemp ret;
  ret.push_back(p);
  if (IMP::atom::Bond::particle_is_instance(p)){
    IMP::atom::Bond b(p);
    ret.push_back(b.get_bonded(0));
    ret.push_back(b.get_bonded(1));
  }
  return ret;
}

ContainersTemp
CommonEndpointPairFilter::get_input_containers(Particle*) const {
  return ContainersTemp();
}

IMPMISC_END_NAMESPACE
