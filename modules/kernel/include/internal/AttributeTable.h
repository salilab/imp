/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ATTRIBUTE_TABLE_H
#define IMPKERNEL_ATTRIBUTE_TABLE_H

#include "../base_types.h"
#include "../utility.h"
#include <IMP/log.h>
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include "../particle_index.h"
#include <boost/dynamic_bitset.hpp>

#include <limits>

IMPKERNEL_BEGIN_NAMESPACE

class Particle;

IMPKERNEL_END_NAMESPACE

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** Base class for defining traits of attributes in the attribute table
    to be stored in a Model object. The traits define the type of attribute
    values, the attribute key, a container for the list of values etc.

    Template params:
    T - the attribute type
    K - the attribute key type
*/
template <class T, class K>
struct DefaultTraits {
  //! a container storing the attribute data for all particles (indexed by ParticleIndex)
  typedef IndexVector<ParticleIndexTag, T> Container;
  typedef T Value;
  typedef T PassValue;
  typedef K Key;
  typedef PassValue const* ContainerConstDataAccess;
  typedef Value* ContainerDataAccess;
  //! returns the maximum of a and b
  static const T &max(const T &a, const T &b) { return std::max(a, b); }
  //! returns the minimum of a and b
  static const T &min(const T &a, const T &b) { return std::min(a, b); }
  //! allow direct const access to the container data
  static ContainerConstDataAccess access_container_data(Container const& c) { return c.data(); }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c.data(); }
};

template <class T, class K>
struct ArrayTraits {
  typedef IMP::Vector<T> Value;
  typedef IndexVector<ParticleIndexTag, Value> Container;
  typedef const Value &PassValue;
  typedef K Key;
  typedef Value const* ContainerConstDataAccess;
  typedef Value* ContainerDataAccess;
  static Value get_invalid() { return Value(); }
  static bool get_is_valid(const Value &v) { return !v.empty(); }
  static const Value &max(const Value &a, const Value &) { return a; }
  static const Value &min(const Value &, const Value &b) { return b; }
  //! allow direct const access to the container data
  static ContainerConstDataAccess access_container_data(Container const& c) { return c.data(); }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c.data(); }
};

//! traits for a table of Float attribute (a C/C++ double type),
//! see also DefaultTraits
struct FloatAttributeTableTraits : public DefaultTraits<double, FloatKey> {
  static double get_invalid() {
    /* do not use NaN as sometimes GCC will optimize things incorrectly.*/
    /*if (std::numeric_limits<float>::has_quiet_NaN) {
         return std::numeric_limits<float>::quiet_NaN();
         } else*/
    if (std::numeric_limits<double>::has_infinity) {
      return std::numeric_limits<double>::infinity();
    } else {
      return std::numeric_limits<double>::max();
    }
  }
  static bool get_is_valid(double f) {
    /*if (std::numeric_limits<float>::has_quiet_NaN) {
      return !isnan(f);
      } else*/
    return f < std::numeric_limits<double>::max();
  }
  //  //! allow direct const access to the container data
  //  static double const* access_container_data(Container const& c) { return c.data(); }
  //  //! allow direct non-const access to the container data
  //  static double*       access_container_data(Container&       c) { return c.data(); }

};

struct ParticleAttributeTableTraits
    : public DefaultTraits<ParticleIndex, ParticleIndexKey> {
  static Value get_invalid() {
    return get_invalid_index<ParticleIndexTag>();
  }
  static bool get_is_valid(const Value &f) { return f != get_invalid(); }
};

struct ParticlesAttributeTableTraits
    : public ArrayTraits<ParticleIndex, ParticleIndexesKey> {};

struct ObjectAttributeTableTraits {
  typedef Object *Value;
  typedef Object *PassValue;
  typedef ObjectKey Key;
  typedef IndexVector<ParticleIndexTag, Pointer<Object> >
      Container;
  typedef Pointer<Object> const* ContainerConstDataAccess;
  typedef Pointer<Object>* ContainerDataAccess;
  static Value get_invalid() { return nullptr; }
  static bool get_is_valid(const Value &f) { return f; }
  static Value min(Value a, Value b) { return std::min(a, b); }
  static Value max(Value a, Value b) { return std::max(a, b); }
    //! allow direct const access to the container data
  static ContainerConstDataAccess access_container_data(Container const& c) { return c.data(); }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c.data(); }

};

struct WeakObjectAttributeTableTraits {
  typedef Object *Value;
  typedef Object *PassValue;
  typedef WeakObjectKey Key;
  typedef IndexVector<ParticleIndexTag, WeakPointer<Object> >
      Container;
  typedef WeakPointer<Object> const* ContainerConstDataAccess;
  typedef WeakPointer<Object>* ContainerDataAccess;
  static Value get_invalid() { return nullptr; }
  static bool get_is_valid(const Value &f) { return f; }
  static Value min(Value a, Value b) { return std::min(a, b); }
  static Value max(Value a, Value b) { return std::max(a, b); }
    //! allow direct const access to the container data
  static ContainerConstDataAccess access_container_data(Container const& c) { return c.data(); }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c.data(); }
};

struct ObjectsAttributeTableTraits {
  typedef Objects Value;
  typedef const Objects &PassValue;
  typedef ObjectsKey Key;
  typedef IndexVector<ParticleIndexTag, Objects> Container;
  typedef Objects const* ContainerConstDataAccess;
  typedef Objects* ContainerDataAccess;
  static Value get_invalid() { return Value(); }
  static bool get_is_valid(const Value &f) { return !f.empty(); }
  static Value min(Value a, Value b) {
    IMP_UNUSED(b);
    return a;
  }
  static Value max(Value a, Value b) {
    IMP_UNUSED(a);
    return b;
  }
  //! allow direct const access to the container data
  static ContainerConstDataAccess access_container_data(Container const& c) { return c.data(); }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c.data(); }
};

struct IntsAttributeTableTraits : ArrayTraits<int, IntsKey> {};

struct IntAttributeTableTraits : public DefaultTraits<Int, IntKey> {
  static Int get_invalid() { return std::numeric_limits<Int>::max(); }
  static bool get_is_valid(Int f) { return f != get_invalid(); }

};

struct BoolAttributeTableTraits : public DefaultTraits<bool, FloatKey> {
  struct Container : public boost::dynamic_bitset<> {
    typedef boost::dynamic_bitset<> P;
    P::reference operator[](Index<ParticleIndexTag> i) {
      return P::operator[](get_as_unsigned_int(i));
    }
    bool operator[](Index<ParticleIndexTag> i) const {
      return P::operator[](get_as_unsigned_int(i));
    }
  };
  typedef Container const& ContainerConstDataAccess;
  typedef Container& ContainerDataAccess;
  static bool get_invalid() { return false; }
  static bool get_is_valid(bool f) { return f; }
    //! allow direct const access to the container data
  ContainerConstDataAccess access_container_data(Container const& c) { return c; }
  //! allow direct non-const access to the container data
  static ContainerDataAccess access_container_data(Container&       c) { return c; }

};

struct StringAttributeTableTraits : public DefaultTraits<String, StringKey> {
  static Value get_invalid() { return "This is an invalid string in IMP"; }
  static bool get_is_valid(String f) { return f != get_invalid(); }
};

// The traits for the particle class are declared in the Particle.h

/** \internal
    If memory becomes a problem then either use a char table to look up
    actual entries or use perfect hashing.
    http://burtleburtle.net/bob/hash/perfect.html

    Actually Cuckoo hashing is probably a better bet as that gets
    high occupancy without large tables for the hash function.

    \note This version of the table uses certain values of the data
    to signal that the entry is invalid. Setting an entry to these
    values is a checked error. The values are specified by the
    Traits::invalid entry.
 */

namespace {
static const FloatKey xyzr_keys[] = {FloatKey(0U), FloatKey(1U),
                                     FloatKey(2U), FloatKey(3U)};

#ifndef __clang__
// just to disable a warning
inline int use_xyz_to_disable_warning() {
  if (xyzr_keys[0] != FloatKey(0U)) {
    return 1;
  } else
    return 0;
}
#endif
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_ATTRIBUTE_TABLE_H */
