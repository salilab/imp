/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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
template <class Storage>
class AttributeTable
{
  typedef AttributeTable<Storage> This;
  typedef typename Storage::Traits ValueTraits;

  Storage map_;

  void check_contains(typename ValueTraits::Key k) const {
    IMP_check(map_.fits(k.get_index())
              && ValueTraits::get_is_valid(map_.get(k.get_index())),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException);
  }
public:
  typedef typename ValueTraits::Value Value;
  typedef typename ValueTraits::PassValue PassValue;
  typedef typename ValueTraits::Key Key;
  AttributeTable(Storage s= Storage()): map_(s){}

  void clear() {
    map_.clear();
  }

  const PassValue get_value(Key k) const {
    check_contains(k);
    return map_.get(k.get_index());
  }
  unsigned int get_length() const {
    return map_.get_length();
  }


  void set_value(Key k, PassValue v) {
    check_contains(k);
    Value vv(v);
    IMP_check(ValueTraits::get_is_valid(vv),
              "Cannot set value of attribute to " << v,
              ValueException);
    map_.set(k.get_index(), vv);
  }

  void set_values(PassValue v) {
    for (unsigned int i=0; i< map_.size(); ++i) {
      map_.set(i,v);
    }
  }

  void insert(Key k, PassValue v) {
    IMP_assert(!contains(k),
               "Trying to add attribute \"" << k.get_string()
               << "\" twice");
    insert_always(k, v);
  }

  void insert_always(Key k, PassValue v) {
    /*std::cout << "Insert " << k << " in v of size "
      << size_ << " " << map_ << " " << k.get_index() << std::endl;*/
    IMP_assert(k != Key(),
               "Can't insert default key");
    Value vv(v);
    IMP_check(ValueTraits::get_is_valid(vv),
              "Trying to insert invalid value for attribute "
              << v << " into attribute " << k,
              ValueException);
    unsigned int val
      =static_cast<unsigned int>(k.get_index());
    if (0) std::cout << val;
    IMP_assert(val <100000, "Bad key index: " << k.get_index()
               << " " << k.get_string());
    map_.add(k.get_index(), vv);
  }

  void remove(Key k) {
    check_contains(k);
    remove_always(k);
  }

  void remove_always(Key k) {
    IMP_assert(k != Key(), "Can't remove invalid key");
    if (k.get_index() < map_.get_length()) {
      map_.set(k.get_index(), ValueTraits::get_invalid());
      // really, no good reason to shrink
    }
  }


  bool contains(Key k) const {
    IMP_assert(k != Key(), "Can't search for default key");
    return k.get_index() < map_.get_length()
      && ValueTraits::get_is_valid(map_.get(k.get_index()));
  }


  void show(std::ostream &out) const {
    for (unsigned int i=0; i< map_.get_length(); ++i) {
      if (ValueTraits::get_is_valid(map_.get(i))) {
        out << Key(i).get_string() << ": ";
        out << map_.get(i);
        out << std::endl;
      }
    }
  }

  void swap_with( AttributeTable<Storage> &o) {
    swap(map_, o.map_);
  }

};

IMP_OUTPUT_OPERATOR_1(AttributeTable)

IMP_SWAP_1(AttributeTable);

namespace {
  static const FloatKey xyzr_keys[]={FloatKey(0U), FloatKey(1U),
                                     FloatKey(2U), FloatKey(3U)};
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_ATTRIBUTE_TABLE_H */
