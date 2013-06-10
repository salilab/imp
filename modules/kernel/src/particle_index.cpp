/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/particle_index.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/Decorator.h"
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

ParticleIndexAdaptor::ParticleIndexAdaptor(Particle *p):
    ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(base::WeakPointer<Particle> p):
    ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(base::Pointer<Particle> p):
    ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(const Decorator &d):
    ParticleIndex(d.get_particle_index()) {}
IMPKERNEL_END_NAMESPACE
