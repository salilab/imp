/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/static.h"
#include "IMP/Particle.h"
#include "IMP/base/set.h"

IMP_BEGIN_INTERNAL_NAMESPACE

// particle validation
typedef bool (*ParticleFunction)(Particle*);
base::Vector<std::pair<ParticleFunction, ParticleFunction> >
particle_validators;

IMP_END_INTERNAL_NAMESPACE
