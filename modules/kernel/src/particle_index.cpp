/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/particle_index.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/Decorator.h"
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

ParticleIndexAdaptor::ParticleIndexAdaptor(Particle *p)
    : ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(WeakPointer<Particle> p)
    : ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(Pointer<Particle> p)
    : ParticleIndex(p->get_index()) {}

ParticleIndexAdaptor::ParticleIndexAdaptor(const Decorator &d)
    : ParticleIndex(d.get_particle_index()) {}

ParticleIndexesAdaptor::ParticleIndexesAdaptor(const Particles &ps)
    : tmp_(new ParticleIndexes(ps.size())), val_(tmp_.get()) {
  *tmp_ = get_indexes(ParticlesTemp(ps.begin(), ps.end()));
}

ParticleIndexesAdaptor::ParticleIndexesAdaptor(const ParticlesTemp &ps)
    : tmp_(new ParticleIndexes(ps.size())), val_(tmp_.get()) {
  *tmp_ = get_indexes(ps);
}

ParticleIndexPairsAdaptor::ParticleIndexPairsAdaptor(
    const ParticlePairsTemp &ps)
    : ParticleIndexPairs(get_indexes(ps)) {}

namespace {
/* Check to make sure that IMP_FOREACH compiles correctly */
void foreach_test() IMP_UNUSED_FUNCTION;
void foreach_test() {
  ParticleIndexes pis;
  ParticleIndexesAdaptor pia(pis);
  IMP_FOREACH(ParticleIndex pi, pia) { IMP_UNUSED(pi); }
}
}
IMPKERNEL_END_NAMESPACE
