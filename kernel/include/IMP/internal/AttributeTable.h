/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ATTRIBUTE_TABLE_H
#define __IMP_ATTRIBUTE_TABLE_H

#include "../base_types.h"
#include "../utility.h"
#include "../log.h"
#include "../Pointer.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/scoped_array.hpp>

#include <vector>
#include <limits>

namespace IMP
{


namespace internal
{

struct FloatAttributeTableTraits
{
  typedef Float Value;
  typedef KeyBase<Float> Key;
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
      return f!= get_invalid();
    }
  }
};

struct IntAttributeTableTraits
{
  typedef Int Value;
  typedef KeyBase<Int> Key;
  static Int get_invalid() {
    return std::numeric_limits<Int>::max();
  }
  static bool get_is_valid(Int f) {
    return f!= get_invalid();
  }
};

struct BoolAttributeTableTraits
{
  typedef bool Value;
  typedef KeyBase<Float> Key;
  static bool get_invalid() {
    return false;
  }
  static bool get_is_valid(Int f) {
    return f;
  }
};

struct StringAttributeTableTraits
{
  typedef String Value;
  typedef KeyBase<String> Key;
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

  typedef boost::scoped_array<typename Traits::Value> Map; 
  Map map_;
  unsigned int size_;

  typename Traits::Value* new_array(unsigned int asz) const {
    typename Traits::Value* ret= new typename Traits::Value[asz];
    for (unsigned int i=0; i< asz; ++i) {
      ret[i]= Traits::get_invalid();
    }
    return ret;
  }

  void copy_from(unsigned int len, const Map &o) {
    IMP_assert(map_, "Internal error in attribute table");
    IMP_assert(size_ >= len, "Table too small");
    //std::cout << "Copy from " << o << " to " << map_ << std::endl;
    for (unsigned int i=0; i< len; ++i) {
      map_[i]= o[i];
    }
  }

  void realloc(unsigned int olen, const Map &o, unsigned int nlen) {
    //std::cout << "Realloc from " << size_ << " " << map_ << " ";
    if (nlen==0) {
      size_=0;
      map_.reset();
    } else {
      size_=std::max(nlen, 6U);
      map_.reset(new_array(size_));
      copy_from(olen, o);
    }
    //std::cout << " to " << size_ << " " << map_ << std::endl;
  }

  void check_contains(typename Traits::Key k) const {
    IMP_assert(size_ > k.get_index(),
               "Attribute \"" << k.get_string()
               << "\" not found in table.");
    IMP_check(Traits::get_is_valid(map_[k.get_index()]),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException);
  }

public:
  typedef typename Traits::Value Value;
  typedef typename Traits::Key Key;
  AttributeTable(): size_(0){}
  AttributeTable(const This &o): size_(0) {
    //std::cout << "Copy constructor called from " << o.map_ << std::endl;
    realloc(o.size_, o.map_, o.size_);
  }
  ~AttributeTable() {
    //std::cout << "Deleting " << map_ << std::endl; 
  }
  This &operator=(const This &o) {
    //std::cout << "Operator= called from " << o.map_ << std::endl;
    if (&o == this) {
      //std::cout << "Self assignment" << std::endl;
      return *this;
    }
    realloc(o.size_, o.map_, o.size_);
    return *this;
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


  void insert(Key k, Value v) {
    IMP_assert(!contains(k),
               "Trying to add attribute \"" << k.get_string()
               << "\" twice");
    insert_always(k, v);
  }

  void insert_always(Key k, Value v);

  void remove(Key k) {
    check_contains(k);
    map_[k.get_index()]= Traits::get_invalid();
  }

  void remove_always(Key k) {
    if (k.get_index() < size_) {
      map_[k.get_index()]= Traits::get_invalid();
    }
  }


  bool contains(Key k) const {
    IMP_assert(k != Key(), "Can't search for default key");
    return k.get_index() < size_
      && Traits::get_is_valid(map_[k.get_index()]);
  }


  void show(std::ostream &out, const char *prefix="") const;


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

  typedef boost::counting_iterator<Key, boost::random_access_traversal_tag,
                                   std::size_t> KeyIterator;
  typedef boost::filter_iterator<IsAttribute, KeyIterator> AttributeKeyIterator;

  AttributeKeyIterator attribute_keys_begin() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(0U)),
                                KeyIterator(Key(size_)));
  }
  AttributeKeyIterator attribute_keys_end() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(size_)),
                                KeyIterator(Key(size_)));
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
  if (size_ <= k.get_index()) {
    boost::scoped_array<Value> old;
    swap(old, map_);
    realloc(size_, old, k.get_index()+1);
  }
  map_[k.get_index()]= v;
}



template <class Traits>
inline void AttributeTable<Traits>::show(std::ostream &out,
                                        const char *prefix) const
{
  for (unsigned int i=0; i< size_; ++i) {
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

} // namespace IMP

#endif  /* __IMP_ATTRIBUTE_TABLE_H */
