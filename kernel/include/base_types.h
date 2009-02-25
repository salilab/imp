/**
 *  \file base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_BASE_TYPES_H
#define IMP_BASE_TYPES_H

#include "config.h"
#include "Key.h"

#include <string>
#include <vector>

IMP_BEGIN_NAMESPACE

/** \internal \namespace IMP::internal Implementation details.
 */

//! Basic floating-point value (could be float, double...)
typedef float Float;

//! A pair of floats
typedef std::pair<Float, Float> FloatPair;

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

/* This needs to be here so that both Model and Particle can use Particles */
class Particle;

//! A class which is used for representing collections of particles
/** Documentation for std::vector can be found at as part of the SGI
    stl documentation, among other places
    (http://www.sgi.com/tech/stl/Vector.html).

    When used within Python, IMP::Particles acts like a Python list.
 */
typedef std::vector<Particle*> Particles;

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
