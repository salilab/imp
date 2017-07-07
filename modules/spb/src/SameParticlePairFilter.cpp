/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/spb/SameParticlePairFilter.h"
#include "IMP/core/rigid_bodies.h"

IMPSPB_BEGIN_NAMESPACE

SameParticlePairFilter::SameParticlePairFilter()
    : PairPredicate("PairPredicate %1%") {}

int SameParticlePairFilter::get_value_index(
    IMP::Model *m, const IMP::ParticleIndexPair &pip) const {
  return m->get_particle(pip[0]) == m->get_particle(pip[1]);
}

/*
ParticlesTemp SameParticlePairFilter::get_input_particles( Particle* t) const {
  ParticlesTemp ret;
  ret.push_back(t);
  return ret;
}
*/

IMP::ModelObjectsTemp SameParticlePairFilter::do_get_inputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  // In the API instead of passing a particle *t, wrap it in a single element
  // array and pass to this!
  return (IMP::get_particles(m, pis));
}

IMPSPB_END_NAMESPACE
