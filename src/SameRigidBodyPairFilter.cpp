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

/*
int SameRigidBodyPairFilter::get_value(const ParticlePair &pp)
 const {
     if(!core::RigidMember::get_is_setup(pp[0]) ||
        !core::RigidMember::get_is_setup(pp[1])){return false;}
     else{
      return core::RigidMember(pp[0]).get_rigid_body() ==
             core::RigidMember(pp[1]).get_rigid_body();}
}
*/

int SameRigidBodyPairFilter::get_value_index(kernel::Model *m,const
    kernel::ParticleIndexPair &pip)
 const {
     if(!core::RigidMember::get_is_setup(m->get_particle(pip[0])) ||
        !core::RigidMember::get_is_setup(m->get_particle(pip[1])))
        {return false;}
     else{
      return core::RigidMember(m->get_particle(pip[0])).get_rigid_body() ==
             core::RigidMember(m->get_particle(pip[1])).get_rigid_body();}
}


/*
ParticlesTemp SameRigidBodyPairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  if(core::RigidMember::get_is_setup(t)){
   ret.push_back(core::RigidMember(t).get_rigid_body());
  }
  return ret;
}
*/

kernel::ModelObjectsTemp SameRigidBodyPairFilter::do_get_inputs(
   kernel::Model *m,const kernel::ParticleIndexes &pis) const
{
     ParticlesTemp ret;
     //ret.push_back(kernel::get_particles(m,pis));
   // first push all the particles
   for (unsigned int i = 0; i < pis.size(); ++i) {
       ret.push_back(m->get_particle(pis[i]));
   }

   for (unsigned int i = 0; i < pis.size(); ++i) {
    if (core::RigidMember::get_is_setup(m, pis[i])) {
      ret.push_back(core::RigidMember(m->get_particle(pis[i])).
                get_rigid_body());
      }
    }
    return ret;

}


IMPMEMBRANE_END_NAMESPACE
