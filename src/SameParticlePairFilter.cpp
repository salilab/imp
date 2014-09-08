/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/membrane/SameParticlePairFilter.h"
#include "IMP/core/rigid_bodies.h"

IMPMEMBRANE_BEGIN_NAMESPACE

SameParticlePairFilter
::SameParticlePairFilter():PairPredicate("PairPredicate %1%"){
}

int SameParticlePairFilter::get_value_index(kernel::Model *m,const
    kernel::ParticleIndexPair &pip)
 const {
      return m->get_particle(pip[0]) == m->get_particle(pip[1]);
}

/*
ParticlesTemp SameParticlePairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  return ret;
}
*/

kernel::ModelObjectsTemp SameParticlePairFilter::do_get_inputs(
 kernel::Model *m,const kernel::ParticleIndexes &pis) const
{
   // In the API instead of passing a particle *t, wrap it in a single element
   // array and pass to this!
    return(kernel::get_particles(m,pis));

}


IMPMEMBRANE_END_NAMESPACE
