/**
 *  \file IMP/kernel/particle_index.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_INDEX_H
#define IMPKERNEL_PARTICLE_INDEX_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"

IMPKERNEL_BEGIN_NAMESPACE

class Particle;
class Decorator;

/** Get the indexes from a list of particles.*/
IMPKERNELEXPORT ParticleIndexes get_indexes(const ParticlesTemp &ps);

/** Get the particles from a list of indexes.*/
IMPKERNELEXPORT ParticlesTemp get_particles(Model *m,
                                            const ParticleIndexes &ps);

/** Get the indexes from a list of particle pairs. */
IMPKERNELEXPORT ParticleIndexPairs get_indexes(const ParticlePairsTemp &ps);

/** Take Decorator, Particle or ParticleIndex. */
class IMPKERNELEXPORT ParticleIndexAdaptor
#ifndef SWIG
    // suppress swig warning that doesn't make sense and I can't make go away
    : public ParticleIndex, base::InputAdaptor
#endif
      {
 public:
  ParticleIndexAdaptor(Particle *p);
  ParticleIndexAdaptor(ParticleIndex pi) : ParticleIndex(pi) {}
  ParticleIndexAdaptor(const Decorator &d);
#if !defined(SWIG) && !defined(IMP_DOXYGEN) && !defined(IMP_SWIG_WRAPPER)
  ParticleIndexAdaptor(base::WeakPointer<Particle> p);
  ParticleIndexAdaptor(base::Pointer<Particle> p);
#endif
};

/** Take Decorator, Particle or ParticleIndex. */
class IMPKERNELEXPORT ParticleIndexesAdaptor
#ifndef SWIG
    // suppress swig warning that doesn't make sense and I can't make go away
  : public ParticleIndexes, base::InputAdaptor
#endif
      {
 public:
#if !defined(SWIG)
  template <class PS>
    ParticleIndexesAdaptor(const PS &ps) {
    resize(ps.size());
    for (unsigned int i = 0; i < ps.size(); ++i) {
      operator[](i) = ps[i].get_particle_index();
    }
  }
  ParticleIndexesAdaptor(const Particles &ps);
#endif
  ParticleIndexesAdaptor(const ParticlesTemp &ps);
  ParticleIndexesAdaptor(const ParticleIndexes& pi) : ParticleIndexes(pi) {}
  ParticleIndexesAdaptor() {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_PARTICLE_INDEX_H */
