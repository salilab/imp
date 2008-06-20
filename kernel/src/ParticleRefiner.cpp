/**
 *  \file ParticleRefiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/ParticleRefiner.h"

namespace IMP
{

ParticleRefiner::~ParticleRefiner(){}

Particles ParticleRefiner::get_refined(Particle *p) {
  throw ErrorException("Can't refine");
  return Particles();
}

}
