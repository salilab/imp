/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/particle_index.h"
#include "IMP/kernel/internal/container_helpers.h"
IMPKERNEL_BEGIN_NAMESPACE
ParticleIndexes get_indexes(const ParticlesTemp &ps) {
  return internal::get_index(ps);
}

ParticlesTemp get_particles(Model *m, const ParticleIndexes &ps) {
  return internal::get_particle(m, ps);
}

ParticleIndexPairs get_indexes(const ParticlePairsTemp &ps) {
  return internal::get_index(ps);
}


IMPKERNEL_END_NAMESPACE
