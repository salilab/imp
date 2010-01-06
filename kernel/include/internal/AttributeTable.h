/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_ATTRIBUTE_TABLE_H
#define IMP_ATTRIBUTE_TABLE_H

#include "../base_types.h"
#include "../utility.h"
#include "../log.h"
#include "../Pointer.h"
#include "../macros.h"
#include "../VectorOfRefCounted.h"
#include "attribute_storage.h"

#include <limits>

IMP_BEGIN_NAMESPACE

class Particle;

IMP_END_NAMESPACE


IMP_BEGIN_INTERNAL_NAMESPACE

template <class T, class K >
struct DefaultTraits
{
  typedef T Value;
  typedef T PassValue;
  typedef K Key;
};


struct FloatAttributeTableTraits: public DefaultTraits<float, FloatKey>
{
  static float get_invalid() {
    /* do not use NaN as sometimes GCC will optimize things incorrectly.*/
    /*if (std::numeric_limits<float>::has_quiet_NaN) {
      return std::numeric_limits<float>::quiet_NaN();
      } else*/ if (std::numeric_limits<float>::has_infinity) {
      return std::numeric_limits<float>::infinity();
    } else {
      return std::numeric_limits<float>::max();
    }
  }
  static bool get_is_valid(float f) {
    /*if (std::numeric_limits<float>::has_quiet_NaN) {
      return !is_nan(f);
      } else*/ {
      return f != get_invalid();
    }
  }
};



struct ParticlesAttributeTableTraits
{
  typedef Particle* Value;
  typedef Particle* PassValue;
  typedef ParticleKey Key;
  static Value get_invalid() {
    return NULL;
  }
  static bool get_is_valid(const Value& f) {
    return f!= NULL;
  }
};

struct ObjectsAttributeTableTraits
{
  typedef Object* Value;
  typedef Object* PassValue;
  typedef ObjectKey Key;
  static Value get_invalid() {
    return NULL;
  }
  static bool get_is_valid(const Value& f) {
    return f!= NULL;
  }
};

struct DoubleAttributeTableTraits: public DefaultTraits<double, FloatKey>
{
  static double get_invalid() {
    /* do not use NaN as sometimes GCC will optimize things incorrectly.*/
    /*if (std::numeric_limits<double>::has_quiet_NaN) {
      return std::numeric_limits<double>::quiet_NaN();
      } else*/ if (std::numeric_limits<double>::has_infinity) {
      return std::numeric_limits<double>::infinity();
    } else {
      return std::numeric_limits<double>::max();
    }
  }
  static bool get_is_valid(double f) {
    /*if (std::numeric_limits<double>::has_quiet_NaN) {
      return !is_nan(f);
      } else*/ {
      return f != get_invalid();
    }
  }
};

struct IntAttributeTableTraits: public DefaultTraits<Int, IntKey>
{
  static Int get_invalid() {
    return std::numeric_limits<Int>::max();
  }
  static bool get_is_valid(Int f) {
    return f!= get_invalid();
  }
};

struct BoolAttributeTableTraits: public DefaultTraits<bool, FloatKey>
{
  static bool get_invalid() {
    return false;
  }
  static bool get_is_valid(Int f) {
    return f;
  }
};

struct StringAttributeTableTraits: public DefaultTraits<String, StringKey>
{
  static Value get_invalid() {
    return "This is an invalid string in IMP";
  }
  static bool get_is_valid(String f) {
    return f!= get_invalid();
  }
};



// The traits for the particle class are declared in the Particle.h

/** \internal
    If memory becomes a problem then either use a char table to look up
    actual entries or use perfect hashing.
    http://burtleburtle.net/bob/hash/perfect.html

    Actuall Cuckoo hashing is probably a better bet as that gets
    high occupancy without large tables for the hash function.

    \note This version of the table uses certain values of the data
    to singal that the entry is invalid. Setting an entry to these
    values is a checked error. The values are specified by the
    Traits::invalid entry.
 */


namespace {
  static const FloatKey xyzr_keys[]={FloatKey(0U), FloatKey(1U),
                                     FloatKey(2U), FloatKey(3U)};
  // just to disable a warning
  inline int use_xyz_to_disable_warning() {
    if (xyzr_keys[0] != FloatKey(0U)) {
      return 1;
    } else return 0;
  }
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_ATTRIBUTE_TABLE_H */
