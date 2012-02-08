/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/membrane/SameRigidBodyPairFilter.h"
#include "IMP/core/rigid_bodies.h"

IMPMEMBRANE_BEGIN_NAMESPACE

SameRigidBodyPairFilter
::SameRigidBodyPairFilter(){
}

bool SameRigidBodyPairFilter::get_contains(const ParticlePair &pp)
 const {
     if(!core::RigidMember::particle_is_instance(pp[0]) ||
        !core::RigidMember::particle_is_instance(pp[1])){return false;}
     else{
      return core::RigidMember(pp[0]).get_rigid_body() ==
             core::RigidMember(pp[1]).get_rigid_body();}
}

ParticlesTemp SameRigidBodyPairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  if(core::RigidMember::particle_is_instance(t)){
   ret.push_back(core::RigidMember(t).get_rigid_body());
  }
  return ret;
}

ContainersTemp
SameRigidBodyPairFilter::get_input_containers(Particle*p) const {
  return ContainersTemp(1, core::RigidMember(p).get_rigid_body());
}

void SameRigidBodyPairFilter::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
