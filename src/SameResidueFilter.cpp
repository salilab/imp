/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/membrane/SameResidueFilter.h"
#include "IMP/atom/Hierarchy.h"

IMPMEMBRANE_BEGIN_NAMESPACE

SameResidueFilter
::SameResidueFilter(){
}

bool SameResidueFilter::get_contains_particle_pair(const ParticlePair &pp)
 const {
     return atom::Hierarchy(pp[0]).get_parent() ==
            atom::Hierarchy(pp[1]).get_parent();
}

ParticlesTemp SameResidueFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  ret.push_back(atom::Hierarchy(t).get_parent());
  return ret;
}


ContainersTemp
SameResidueFilter::get_input_containers(Particle*p) const {
  return ContainersTemp(1, atom::Hierarchy(p).get_parent());
}
void SameResidueFilter::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
