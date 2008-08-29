/**
 *  \file base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BASE_TYPES_H
#define __IMP_BASE_TYPES_H

#include "IMP_config.h"
#include "Index.h"
#include "Key.h"

#include <string>
#include <vector>

namespace IMP
{


/** \defgroup kernel IMP Kernel
    The core IMP types

    \defgroup helper Miscellaneous helpers
    Classes and functions which are there to make your life easier.
 */

/** \namespace IMP::internal
    Implementation details.

    \internal
 */

//! Basic floating-point value (could be float, double...)
typedef float Float;

//! Basic integer value
typedef int Int;

//! Basic string value
typedef std::string String;



typedef std::vector<Float> Floats;
typedef std::vector<Int> Ints;
typedef std::vector<String> Strings;



struct ParticleTag {};
struct RestraintTag {};
struct ScoreStateTag {};
struct OptimizerStateTag {};

typedef Index<ParticleTag> ParticleIndex;
typedef Index<RestraintTag> RestraintIndex;
typedef Index<ScoreStateTag> ScoreStateIndex;
typedef Index<OptimizerStateTag> OptimizerStateIndex;

class Particle;
//! A class which is used for representing collections of particles
/**
   We need this to have a uniform return type for python.
   \todo It would be nice to use internal::Vector instead, but that
   is not as pretty for Python.
 */
typedef std::vector<Particle*> Particles;
typedef std::pair<Particle*, Particle*> ParticlePair;
typedef std::vector<ParticlePair> ParticlePairs;


typedef std::vector<ParticleIndex> ParticleIndexes;

class Particle;

//! The type used to identify float attributes in the Particles
IMP_DECLARE_KEY_TYPE(FloatKey, 0);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntKey, 1);
//! The type used to identify string attributes in the Particles
IMP_DECLARE_KEY_TYPE(StringKey, 2);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticleKey, 3);

} // namespace IMP


#endif  /* __IMP_BASE_TYPES_H */
