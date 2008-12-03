/**
 *  \file ParticleFunction.h     \brief Apply a function on a particle
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_PARTICLE_FUNCTION_H
#define IMPMISC_PARTICLE_FUNCTION_H

#include "misc_exports.h"

#include <IMP/Particle.h>

IMPMISC_BEGIN_NAMESPACE

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

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_PARTICLE_FUNCTION_H */
