/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/SameResiduePairFilter.h"
#include "IMP/atom/Hierarchy.h"

IMPATOM_BEGIN_NAMESPACE

SameResiduePairFilter
::SameResiduePairFilter(){
}

bool SameResiduePairFilter::get_contains_particle_pair(const ParticlePair &pp)
 const {
     return Hierarchy(pp[0]).get_parent() ==
            Hierarchy(pp[1]).get_parent();
}

ParticlesTemp SameResiduePairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  ret.push_back(Hierarchy(t).get_parent());
  return ret;
}


ContainersTemp
SameResiduePairFilter::get_input_containers(Particle*p) const {
  return ContainersTemp(1, Hierarchy(p).get_parent());
}
void SameResiduePairFilter::do_show(std::ostream &) const {
}

IMPATOM_END_NAMESPACE
