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

/** Get the indexes from a list of particles.*/
IMPKERNELEXPORT ParticleIndexes get_indexes(const ParticlesTemp &ps);

/** Get the particles from a list of indexes.*/
IMPKERNELEXPORT ParticlesTemp
get_particles(Model *m, const ParticleIndexes &ps);

/** Get the indexes from a list of particle pairs. */
IMPKERNELEXPORT ParticleIndexPairs get_indexes(const ParticlePairsTemp &ps);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_PARTICLE_INDEX_H */
