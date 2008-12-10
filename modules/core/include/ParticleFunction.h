/**
 *  \file ParticleFunction.h     \brief Apply a function on a particle
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_PARTICLE_FUNCTION_H
#define IMPCORE_PARTICLE_FUNCTION_H

#include "config.h"

#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

//! Virtual class for applying a function on Particles
/**
 */
class ParticleFunction
{
public:
  //! Initialize the particle function
  ParticleFunction(){}
  virtual ~ParticleFunction(){}
  //! Apply a function on a particle
  virtual void apply(Particles ps){}
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_PARTICLE_FUNCTION_H */
