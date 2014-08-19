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

int SameRigidBodyPairFilter::get_value(const ParticlePair &pp)
 const {
     if(!core::RigidMember::get_is_setup(pp[0]) ||
        !core::RigidMember::get_is_setup(pp[1])){return false;}
     else{
      return core::RigidMember(pp[0]).get_rigid_body() ==
             core::RigidMember(pp[1]).get_rigid_body();}
}

ParticlesTemp SameRigidBodyPairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  if(core::RigidMember::get_is_setup(t)){
   ret.push_back(core::RigidMember(t).get_rigid_body());
  }
  return ret;
}

ContainersTemp
SameRigidBodyPairFilter::get_input_containers(Particle*p) const {
  return ContainersTemp();
}

void SameRigidBodyPairFilter::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
