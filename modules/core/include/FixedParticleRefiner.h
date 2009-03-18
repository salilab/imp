/**
 *  \file FixedParticleRefiner.h
 *  \brief A particle refiner which returns a fixed set of particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_FIXED_PARTICLE_REFINER_H
#define IMPCORE_FIXED_PARTICLE_REFINER_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/ParticleRefiner.h>
#include <IMP/container_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! The refiner can refine any particle by returning a fixed set
/**
 */
class IMPCOREEXPORT FixedParticleRefiner: public ParticleRefiner
{
  IMP_LIST(private, Particle, particle, Particle*);
public:
  //! Store the set of particles
  FixedParticleRefiner(const Particles &ps);

  virtual ~FixedParticleRefiner();

  IMP_PARTICLE_REFINER(internal::version_info)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_FIXED_PARTICLE_REFINER_H */
