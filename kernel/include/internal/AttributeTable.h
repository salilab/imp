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

#include <boost/scoped_array.hpp>

#include <vector>
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

template <class Value>
class VectorStorage {
  typedef std::vector<Value> Map;
  std::vector<Value> map_;
public:
  VectorStorage(Value initial_value){}
  VectorStorage(unsigned int size, Value initial_value): map_(size,
                                                              initial_value){}
  typename Map::const_reference get(unsigned int i) const {
    IMP_check(i < map_.size(), "Out of range traits.", IndexException);
    return map_[i];
  }
  void set(unsigned int i, const Value &v) {
    IMP_check(i < map_.size(), "Out of range traits.", IndexException);
    map_[i]=v;
  }
  void add(unsigned int i, const Value &v, const Value &fill_value) {
    map_.resize(std::max(static_cast<unsigned int>(map_.size()), i+1),
                fill_value);
    map_[i]= v;
  }
  bool fits(unsigned int i) const {
    return map_.size() > i;
  }
  void clear(Value v) {
    map_.clear();
  }
  unsigned int get_length() const {
    return map_.size();
  }

  void swap_with(VectorStorage<Value> &o) {
    std::swap(map_, o.map_);
  }
  void fill(Value v) {
    std::fill(map_.begin(), map_.end(), v);
  }
};


template <class Value>
class ParticlesStorage {
  VectorOfRefCounted<Value> map_;
public:
  ParticlesStorage(Value){}
  ParticlesStorage(unsigned int size, Value): map_(size){}
  Value get(unsigned int i) const {
    IMP_check(fits(i), "Out of range traits.", IndexException);
    return map_[i];
  }
  void set(unsigned int i, Value v) {
    IMP_check(fits(i), "Out of range traits.", IndexException);
    map_.set(i,v);
  }
  void add(unsigned int i, Value v, Value fill) {
    map_.resize(std::max(static_cast<unsigned int>(map_.size()), i+1));
    map_.set(i, v);
  }
  bool fits(unsigned int i) const {
    return map_.size() > i;
  }
  void clear(Value v) {
    map_.clear();
  }
  unsigned int get_length() const {
    return map_.size();
  }

  void swap_with(VectorStorage<Value> &o) {
    std::swap(map_, o.map_);
  }
  void fill(Value v) {
    std::fill(map_.begin(), map_.end(), v);
  }
};


// save a word since we don't need separate capacity
template <class Value>
class ArrayStorage {
  boost::scoped_array<Value> data_;
  unsigned int size_;
public:
  ArrayStorage(const ArrayStorage<Value> &o) {
    operator=(o);
  }
  const ArrayStorage<Value>& operator=(const ArrayStorage<Value> &o) {
    size_= o.size_;
    if (size_>0) {
      data_.reset(new Value[size_]);
      std::copy(o.data_.get(), o.data_.get()+size_, data_.get());
    }
    return *this;
  }
  ArrayStorage(Value initial_value): size_(0){}
  ArrayStorage(unsigned int size, Value initial_value):
    data_(new Value[size]), size_(size){
    fill(initial_value);
  }
  const Value& get(unsigned int i) const {
    IMP_check(fits(i), "Out of range traits.", IndexException);
    return data_[i];
  }
  void set(unsigned int i, const Value &v) {
    IMP_check(fits(i), "Out of range traits.", IndexException);
    data_[i]=v;
  }
  void add(unsigned int i, const Value &v, const Value &fill_value) {
    if (i+1 > size_) {
      boost::scoped_array<Value> n(new Value[i+1]);
      std::copy(data_.get(), data_.get()+size_, n.get());
      std::fill(n.get()+size_, n.get()+i, fill_value); // skip the last
      data_.swap(n);
      size_= i+1;
    }
    data_[i]= v;
  }
  bool fits(unsigned int i) const {
    return size_ > i;
  }
  void clear(Value v) {
    size_=0;
    data_.reset();
  }
  unsigned int get_length() const {
    return size_;
  }

  void swap_with(VectorStorage<Value> &o) {
    std::swap(data_, o.data_);
    std::swap(size_, o.size_);
  }
  void fill(Value v) {
    std::fill(data_.get(), data_.get()+size_, v);
  }
};

template <class Value, unsigned int OFFSET>
class OffsetArrayStorage: public ArrayStorage<Value> {
  typedef ArrayStorage<Value> P;
public:
  OffsetArrayStorage(Value initial_value): P(initial_value){}
  OffsetArrayStorage(unsigned int size, Value initial_value):
    P(size-OFFSET, initial_value) {
    IMP_assert(size >= OFFSET, "Indexes smaller than "
               << OFFSET << " should not make it here.");
  }
  const Value& get(unsigned int i) const {
    return P::get(i-OFFSET);
  }
  void set(unsigned int i, const Value &v) {
    return P::set(i-OFFSET);
  }
  void add(unsigned int i, const Value &v, const Value &fill_value) {
    return P::add(i-OFFSET, v, fill_value);
  }
  bool fits(unsigned int i) const {
    return P::fist(i-OFFSET);
  }
};



template <class Value, int SIZE, class Overflow>
class InlineStorage {
  Value data_[SIZE];
  Overflow overflow_;
public:
  InlineStorage(Value initial_value){
    clear(initial_value);
  }
  InlineStorage(int size, Value initial_value):
    overflow_(std::max(0, size-SIZE),
              initial_value){
    clear(initial_value);
  }
  Value get(unsigned int i) const {
    IMP_check(fits(i), "Out of range attribuite: " << i,
              IndexException);
    if (i< SIZE) { return data_[i];}
    else {return overflow_.get(i-SIZE);}
  }
  void set(unsigned int i, const Value &v) {
    IMP_check(fits(i), "Out of range attribuite: " << i,
              IndexException);
    if (i< SIZE) { data_[i]=v;}
    else {overflow_.set(i-SIZE, v);}
  }
  void add(unsigned int i, const Value &v, const Value &fill_value) {
    if (i >= SIZE) {
      overflow_.add(i-SIZE, v, fill_value);
    }
    set(i, v);
  }
  bool fits(unsigned int i) const {
    if (i < SIZE) return true;
    return overflow_.fits(i-SIZE);
  }
  void clear(Value v) {
    overflow_.clear(v);
    fill(v);
  }
  unsigned int get_length() const {
    return SIZE+overflow_.get_length();
  }

  void swap_with(InlineStorage<Value, SIZE, Overflow> &o) {
    for (unsigned int i=0; i< SIZE; ++i) {
      std::swap(data_[i], o.data_[i]);
    }
    swap(overflow_, o.overflow_);
  }
  void fill(Value v) {
    std::fill(data_, data_+SIZE, v);
    overflow_.fill(v);
  }
};

template <class V, int S, class O>
void swap(InlineStorage<V,S, O> &a,
          InlineStorage<V,S, O> &b) {
  a.swap_with(b);
}


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
template <class ValueTraits, class Storage>
class AttributeTable
{
  typedef AttributeTable<ValueTraits, Storage> This;

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
  AttributeTable(): map_(ValueTraits::get_invalid()){}

  void clear() {
    map_.clear(ValueTraits::get_invalid());
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
    map_.add(k.get_index(), vv,
             ValueTraits::get_invalid());
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

  void swap_with( AttributeTable<ValueTraits, Storage> &o) {
    swap(map_, o.map_);
  }

};

IMP_OUTPUT_OPERATOR_2(AttributeTable)

IMP_SWAP_2(AttributeTable);

namespace {
  static const FloatKey xyzr_keys[]={FloatKey(0U), FloatKey(1U),
                                     FloatKey(2U), FloatKey(3U)};
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_ATTRIBUTE_TABLE_H */
