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

#include <string>
#include <iostream>
#include <vector>

namespace IMP
{


/** \defgroup kernel IMP Kernel
    The core IMP types

    \defgroup helper Miscellaneous helpers
    Classes and functions which are there to make your life easier.
 */

/** \namespace internal
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

typedef Index<Int> IntIndex;
typedef Index<Float> FloatIndex;
typedef Index<String> StringIndex;
typedef Index<ParticleTag> ParticleIndex;
typedef Index<RestraintTag> RestraintIndex;
typedef Index<ScoreStateTag> ScoreStateIndex;
typedef Index<OptimizerStateTag> OptimizerStateIndex;

class Particle;
//! A class which is used for representing collections of particles
/**
   We need this to have a uniform return type for python.
 */
typedef std::vector<Particle*> Particles;
typedef std::pair<Particle*, Particle*> ParticlePair;
typedef std::vector<ParticlePair> ParticlePairs;


typedef std::vector<ParticleIndex> ParticleIndexes;
typedef std::vector<FloatIndex> FloatIndexes;
typedef std::vector<IntIndex> IntIndexes;
typedef std::vector<StringIndex> StringIndexes;
// typedefs for the particle variable and attribute indexes
// typedef DataIndex<Float> FloatIndex;
// typedef DataIndex<Int> IntIndex;
// typedef DataIndex<String> StringIndex;

template <class T> class Key;


/*

  NOTE: for stupid and obscure C++-related reasons, the key
  declarations must go here. 

  In detail, the index function declarations must appear before the data()
  function which uses them.

 */

/**
   Define a new key type. There must be an accompanying IMP_DEFINE_KEY_TYPE.
   \note This macro should only be used down in this file after the 

   \param[in] Name The name for the new type.
   \param[in] Tag A class which is unique for this type. For attributes
   use the type of the attributes. For other Keys, declare an empty
   class with a unique name and use it. 
 */
#define IMP_DECLARE_KEY_TYPE(Name, Tag)                             \
  typedef Key<Tag> Name;                                            \
  namespace internal {                                              \
    IMPDLLEXPORT extern unsigned int next_attribute_table_index_;   \
    IMPDLLEXPORT extern const unsigned int Name##_attribute_table_index_; \
    inline unsigned int attribute_table_index(Tag) {                    \
      return Name##_attribute_table_index_;                             \
    }                                                                   \
  }                                                                     \
  typedef std::vector<Name> Name##s;

/**
   Declare static data necessary for a new key type.
 */
#define IMP_DEFINE_KEY_TYPE(Name, Tag)                                  \
  namespace internal {                                                  \
    const unsigned int                                                  \
    Name##_attribute_table_index_=next_attribute_table_index_++;        \
  }


//! The type used to identify float attributes in the Particles
IMP_DECLARE_KEY_TYPE(FloatKey, Float);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntKey, Int);
//! The type used to identify string attributes in the Particles
IMP_DECLARE_KEY_TYPE(StringKey, String);

struct AtomTypeTag{};
struct ResidueTypeTag{};

//! The type used to identify atom types
IMP_DECLARE_KEY_TYPE(AtomType, AtomTypeTag);
//! The type used to identify residue types
IMP_DECLARE_KEY_TYPE(ResidueType, ResidueTypeTag);



} // namespace IMP

#include "Key.h"

#endif  /* __IMP_BASE_TYPES_H */
