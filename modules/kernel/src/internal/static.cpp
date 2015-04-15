/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/static.h"
#include "IMP/Particle.h"
#include "IMP/internal/utility.h"
#include "IMP/set.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

// particle validation
Vector<std::pair<ParticleFunction, ParticleFunction> >
    particle_validators;

IMPKERNEL_END_INTERNAL_NAMESPACE
