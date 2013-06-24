/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/internal/static.h"
#include "IMP/kernel/Particle.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/base//set.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

// particle validation
base::Vector<std::pair<ParticleFunction, ParticleFunction> >
    particle_validators;

IMPKERNEL_END_INTERNAL_NAMESPACE
