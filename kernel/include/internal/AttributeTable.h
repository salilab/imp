/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_ATTRIBUTE_TABLE_H
#define IMP_ATTRIBUTE_TABLE_H

#include "../base_types.h"
#include "../utility.h"
#include "../log.h"
#include "../Pointer.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/scoped_array.hpp>

#include <vector>
#include <limits>

IMP_BEGIN_NAMESPACE

namespace internal
{

template <class T, class K >
struct DefaultTraits
{
  typedef T Value;
  typedef K Key;
};

struct FloatAttributeTableTraits: public DefaultTraits<Float, FloatKey>
{
  static Float get_invalid() {
    if (std::numeric_limits<Float>::has_quiet_NaN) {
      return std::numeric_limits<Float>::quiet_NaN();
    } else if (std::numeric_limits<Float>::has_infinity) {
      return std::numeric_limits<Float>::infinity();
    } else {
      return std::numeric_limits<Float>::max();
    }
  }
  static bool get_is_valid(Float f) {
    if (std::numeric_limits<Float>::has_quiet_NaN) {
      return f==f;
    } else {
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
template <class Traits>
class AttributeTable
{
  typedef AttributeTable<Traits> This;

  typedef std::vector<typename Traits::Value> Map;
  Map map_;

  void check_contains(typename Traits::Key k) const {
    IMP_assert(map_.size() > k.get_index(),
               "Attribute \"" << k.get_string()
               << "\" not found in table.");
    IMP_check(Traits::get_is_valid(map_[k.get_index()]),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException);
  }

  void clear() {
    map_.clear();
  }

public:
  typedef typename Traits::Value Value;
  typedef typename Traits::Key Key;
  AttributeTable(){}
  ~AttributeTable() {
    //std::cout << "Deleting " << map_ << std::endl;
  }

  const Value get_value(Key k) const {
    check_contains(k);
    return map_[k.get_index()];
  }


  void set_value(Key k, Value v) {
    check_contains(k);
    IMP_check(Traits::get_is_valid(v),
              "Cannot set value of attribute to " << v,
              ValueException);
    map_[k.get_index()] = v;
  }

  void set_values(Value v) {
    for (unsigned int i=0; i< map_.size(); ++i) {
      map_[i]=v;
    }
  }

  void insert(Key k, Value v) {
    IMP_assert(!contains(k),
               "Trying to add attribute \"" << k.get_string()
               << "\" twice");
    insert_always(k, v);
  }

  void insert_always(Key k, Value v);

  void remove(Key k) {
    check_contains(k);
    remove_always(k);
  }

  void remove_always(Key k) {
    IMP_assert(k != Key(), "Can't remove invalid key");
    if (k.get_index() < map_.size()) {
      map_[k.get_index()]= Traits::get_invalid();
      // Cleanup loop. Changed in r709 from a simple pop_back() loop, since that
      // crashed MSVC builds - possibly a hidden corruption bug elsewhere
      // in the code (as that code appears valid).
      long i = map_.size() - 1;
      while (i >= 0 && map_[i] == Traits::get_invalid()) {
        --i;
      }
      map_.erase(map_.begin() + i + 1, map_.end());
    }
  }


  bool contains(Key k) const {
    IMP_assert(k != Key(), "Can't search for default key");
    return k.get_index() < map_.size()
      && Traits::get_is_valid(map_[k.get_index()]);
  }


  void show(std::ostream &out, const std::string prefix="") const;


  std::vector<Key> get_keys() const;

  class IsAttribute
  {
    const This *map_;
  public:
    IsAttribute(): map_(NULL){}
    IsAttribute(const This *map): map_(map) {}
    bool operator()(Key k) const {
      return map_->contains(k);
    }
  };

  typedef boost::counting_iterator<Key, boost::forward_traversal_tag,
                                   unsigned int> KeyIterator;
  typedef boost::filter_iterator<IsAttribute, KeyIterator> AttributeKeyIterator;

  AttributeKeyIterator attribute_keys_begin() const {
    KeyIterator b(0U);
    KeyIterator e(map_.size());
    IMP_assert(std::distance(b,e)
               == map_.size(), "Something is broken with the iterators");
    IMP_assert(std::distance(AttributeKeyIterator(IsAttribute(this), b,e),
                             AttributeKeyIterator(IsAttribute(this), e,e))
                             <= map_.size(), "Broken in filter");
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(0U)),
                                KeyIterator(Key(map_.size())));
  }
  AttributeKeyIterator attribute_keys_end() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(map_.size())),
                                KeyIterator(Key(map_.size())));
  }

};

IMP_OUTPUT_OPERATOR_1(AttributeTable)




template <class Traits>
inline void AttributeTable<Traits>::insert_always(Key k, Value v)
{
  /*std::cout << "Insert " << k << " in v of size "
    << size_ << " " << map_ << " " << k.get_index() << std::endl;*/
  IMP_assert(k != Key(),
            "Can't insert default key");
  IMP_check(Traits::get_is_valid(v),
            "Trying to insert invalid value for attribute "
            << v << " into attribute " << k,
            ValueException);
  typename Map::size_type val
    =static_cast<typename Map::size_type>(k.get_index());
  IMP_assert(val <100000, "Bad key index: " << k.get_index()
             << " " << k.get_string());
  map_.resize(std::max(map_.size(),
                       val+1),
              Traits::get_invalid());
  map_[k.get_index()]= v;
}



template <class Traits>
inline void AttributeTable<Traits>::show(std::ostream &out,
                                         const std::string prefix) const
{
  for (unsigned int i=0; i< map_.size(); ++i) {
    if (Traits::get_is_valid(map_[i])) {
      out << prefix;
      out << Key(i).get_string() << ": ";
      out << map_[i];
      out << std::endl;
    }
  }
}


template <class Traits>
inline std::vector<typename Traits::Key>
  AttributeTable<Traits>::get_keys() const
{
  std::vector<Key> ret(attribute_keys_begin(), attribute_keys_end());
  return ret;
}


} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_ATTRIBUTE_TABLE_H */
