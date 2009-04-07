/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Refiner.h"

IMP_BEGIN_NAMESPACE

const Particles Refiner::get_refined(Particle *p) const {
  return Particles(refined_begin(p), refined_end(p));
}

IMP_END_NAMESPACE
