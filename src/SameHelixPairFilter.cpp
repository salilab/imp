/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/membrane/SameHelixPairFilter.h"
#include "IMP/core/rigid_bodies.h"

IMPMEMBRANE_BEGIN_NAMESPACE

SameHelixPairFilter
::SameHelixPairFilter(){
}

bool SameHelixPairFilter::get_contains(const ParticlePair &pp)
 const {
     return core::RigidMember(pp[0]).get_rigid_body() ==
            core::RigidMember(pp[1]).get_rigid_body();
}

ParticlesTemp SameHelixPairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  ret.push_back(core::RigidMember(t).get_rigid_body());
  return ret;
}


ContainersTemp
SameHelixPairFilter::get_input_containers(Particle*p) const {
  return ContainersTemp(1, core::RigidMember(p).get_rigid_body());
}
void SameHelixPairFilter::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
