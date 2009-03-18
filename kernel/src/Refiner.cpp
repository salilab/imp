/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Refiner.h"

IMP_BEGIN_NAMESPACE

Particles Refiner::get_refined(Particle *p) const {
  throw ErrorException("Can't refine");
  return Particles();
}

IMP_END_NAMESPACE
