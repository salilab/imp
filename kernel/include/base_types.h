/**
 *  \file base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_BASE_TYPES_H
#define IMP_BASE_TYPES_H

#include "IMP_config.h"
#include "Index.h"
#include "Key.h"

#include <string>
#include <vector>

IMP_BEGIN_NAMESPACE

/** \internal \namespace IMP::internal Implementation details.
 */

//! Basic floating-point value (could be float, double...)
typedef float Float;

//! Basic integer value
typedef int Int;

//! Basic string value
typedef std::string String;


//! Standard way to pass a bunch of Float values
typedef std::vector<Float> Floats;
//! Standard way to pass a bunch of Int values
typedef std::vector<Int> Ints;
//! Standard way to pass a bunch of String values
typedef std::vector<String> Strings;



struct ParticleTag {};
struct RestraintTag {};
struct ScoreStateTag {};
struct OptimizerStateTag {};

//! Index to access a Particle in a container in an object
typedef Index<ParticleTag> ParticleIndex;
//! Index to access a Restraint in a container in an object
typedef Index<RestraintTag> RestraintIndex;
//! Index to access a ScoreState in a container in an object
typedef Index<ScoreStateTag> ScoreStateIndex;
//! Index to access a OptimizerState in a container in an object
typedef Index<OptimizerStateTag> OptimizerStateIndex;

class Particle;
//! A class which is used for representing collections of particles
/**
   We need this to have a uniform return type for python.
   \todo It would be nice to use internal::Vector instead, but that
   is not as pretty for Python.
 */
typedef std::vector<Particle*> Particles;
//! The standard way of storing a pair of Particles
typedef std::pair<Particle*, Particle*> ParticlePair;
//! A collection of ParticlePair
typedef std::vector<ParticlePair> ParticlePairs;

//! A collection of ParticleIndex
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

IMP_END_NAMESPACE

#endif  /* IMP_BASE_TYPES_H */
