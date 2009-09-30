/**
 *  \file internal/particle.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_ATTRIBUTE_STORAGE_H
#define IMP_INTERNAL_ATTRIBUTE_STORAGE_H

#include "../config.h"
#include "../base_types.h"

#include <boost/scoped_array.hpp>
#include <vector>


IMP_BEGIN_INTERNAL_NAMESPACE
template <class TraitsT>
class VectorStorage {
  typedef std::vector<typename TraitsT::Value> Map;
  Map map_;
public:
  typedef TraitsT Traits;
  VectorStorage(){}
  VectorStorage(unsigned int size): map_(size,
                                         Traits::get_invalid()){}
  typename Map::const_reference get(unsigned int i) const {
    IMP_assert(i < map_.size(), "Out of range traits.");
    return map_[i];
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    IMP_assert(i < map_.size(), "Out of range traits.");
    map_[i]=v;
  }
  void add(unsigned int i,  typename Traits::PassValue v) {
    map_.resize(std::max(static_cast<unsigned int>(map_.size()), i+1),
                Traits::get_invalid());
    map_[i]= v;
  }
  void remove(unsigned int i) {
    map_[i]= Traits::get_invalid();
  }
  bool fits(unsigned int i) const {
    return map_.size() > i;
  }
  void clear() {
    map_.clear();
  }
  unsigned int get_length() const {
    return map_.size();
  }

  void swap_with(VectorStorage<Traits> &o) {
    std::swap(map_, o.map_);
  }
  void fill(typename Traits::PassValue v) {
    std::fill(map_.begin(), map_.end(), v);
  }
};


IMP_SWAP_1(VectorStorage);

template <class TraitsT>
class RefCountedStorage {
  VectorOfRefCounted<typename TraitsT::Value> map_;
public:
  typedef TraitsT Traits;
  RefCountedStorage(){}
  RefCountedStorage(unsigned int size): map_(size){}
  typename Traits::PassValue get(unsigned int i) const {
    IMP_assert(fits(i), "Out of range traits.");
    return map_[i];
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    IMP_assert(fits(i), "Out of range traits.");
    map_.set(i,v);
  }
  void add(unsigned int i, typename Traits::PassValue v) {
    map_.resize(std::max(static_cast<unsigned int>(map_.size()), i+1));
    map_.set(i, v);
  }
  void remove(unsigned int i) {
    map_.set(i, NULL);
  }
  bool fits(unsigned int i) const {
    return map_.size() > i;
  }
  void clear() {
    map_.clear();
  }
  unsigned int get_length() const {
    return map_.size();
  }

  void swap_with(VectorStorage<Traits> &o) {
    std::swap(map_, o.map_);
  }
  void fill(typename Traits::PassValue v) {
    std::fill(map_.begin(), map_.end(), v);
  }
};


// save a word since we don't need separate capacity
template <class TraitsT>
class ArrayStorage {
  boost::scoped_array<typename TraitsT::Value> data_;
  unsigned int size_;
public:
  typedef TraitsT Traits;
  ArrayStorage(const ArrayStorage<Traits> &o) {
    operator=(o);
  }
  const ArrayStorage<Traits>& operator=(const ArrayStorage<Traits> &o) {
    size_= o.size_;
    if (size_>0) {
      data_.reset(new typename Traits::Value[size_]);
      std::copy(o.data_.get(), o.data_.get()+size_, data_.get());
    }
    return *this;
  }
  ArrayStorage(): size_(0){}
  ArrayStorage(unsigned int size):
    data_(new typename Traits::Value[size]), size_(size){
    fill(Traits::get_invalid());
  }
  typename Traits::PassValue get(unsigned int i) const {
    IMP_assert(fits(i), "Out of range traits.");
    return data_[i];
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    IMP_assert(fits(i), "Out of range traits.");
    data_[i]=v;
  }
  void add(unsigned int i, typename Traits::PassValue v) {
    if (i+1 > size_) {
      boost::scoped_array<typename Traits::Value>
        n(new typename Traits::Value[i+1]);
      std::copy(data_.get(), data_.get()+size_, n.get());
      std::fill(n.get()+size_, n.get()+i,
                Traits::get_invalid()); // skip the last
      data_.swap(n);
      size_= i+1;
    }
    data_[i]= v;
  }
  void remove(unsigned int i) {
    if (fits(i)) {
      set(i, Traits::get_invalid());
    }
  }
  bool fits(unsigned int i) const {
    return size_ > i;
  }
  void clear() {
    size_=0;
    data_.reset();
  }
  unsigned int get_length() const {
    return size_;
  }

  void swap_with(VectorStorage<Traits> &o) {
    std::swap(data_, o.data_);
    std::swap(size_, o.size_);
  }
  void fill(typename Traits::Value v) {
    std::fill(data_.get(), data_.get()+size_, v);
  }
};

template <class P, unsigned int OFFSET>
class OffsetStorage: public P {
public:
  typedef typename P::Traits Traits;
  OffsetStorage(){}
  OffsetStorage(unsigned int size):
    P(size-OFFSET) {
    IMP_assert(size >= OFFSET, "Indexes smaller than "
               << OFFSET << " should not make it here.");
  }
  typename Traits::PassValue get(unsigned int i) const {
    return P::get(i-OFFSET);
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    return P::set(i-OFFSET, v);
  }
  void add(unsigned int i, typename Traits::PassValue v) {
    return P::add(i-OFFSET, v);
  }
  void remove(unsigned int i) {
    P::remove(i-OFFSET);
  }
  bool fits(unsigned int i) const {
    return P::fits(i-OFFSET);
  }
  unsigned int get_length() const {
    return OFFSET+P::get_length();
  }
};


template <class TraitsT, int SIZE>
class FixedInlineStorage {
  typename TraitsT::Value data_[SIZE];
public:
  typedef TraitsT Traits;
  FixedInlineStorage(){
    clear();
  }
  FixedInlineStorage(int size) {
    clear();
  }
  typename Traits::PassValue get(unsigned int i) const {
    IMP_assert(fits(i), "Out of range attribuite: " << i);
    return data_[i];
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    IMP_assert(fits(i), "Out of range attribuite: " << i);
    data_[i]=v;
  }
  void add(unsigned int i, typename Traits::PassValue v) {
    set(i, v);
  }
  void remove(unsigned int i) {
    set(i, Traits::get_invalid());
  }
  bool fits(unsigned int i) const {
    return (i < SIZE);
  }
  void clear() {
    fill(Traits::get_invalid());
  }
  unsigned int get_length() const {
    return SIZE;
  }

  void swap_with(FixedInlineStorage<Traits, SIZE> &o) {
    for (unsigned int i=0; i< SIZE; ++i) {
      std::swap(data_[i], o.data_[i]);
    }
  }
  void fill(typename Traits::PassValue v) {
    std::fill(data_, data_+SIZE, v);
  }
};

template <class V, int S>
void swap(FixedInlineStorage<V,S> &a,
          FixedInlineStorage<V,S> &b) {
  a.swap_with(b);
}



template <class Base, int SIZE>
class InlineStorage: public FixedInlineStorage<typename Base::Traits,
                                               SIZE> {
  typedef FixedInlineStorage<typename Base::Traits, SIZE> P;
  Base overflow_;
public:
  typedef typename Base::Traits Traits;
  InlineStorage(){
  }
  InlineStorage(int size):
    overflow_(std::max(0, size-SIZE)){
  }
  typename Traits::PassValue get(unsigned int i) const {
    IMP_assert(fits(i), "Out of range attribuite: " << i);
    if (i< SIZE) { return P::get(i);}
    else {return overflow_.get(i-SIZE);}
  }
  void set(unsigned int i, typename Traits::PassValue v) {
    IMP_assert(fits(i), "Out of range attribuite: " << i);
    if (i< SIZE) { P::set(i, v);}
    else {overflow_.set(i-SIZE, v);}
  }
  void add(unsigned int i, typename Traits::PassValue v) {
    if (i >= SIZE) {
      overflow_.add(i-SIZE, v);
    }
    set(i, v);
  }
  void remove(unsigned int i) {
    if (i >= SIZE) {
      overflow_.remove(i-SIZE);
    } else {
      P::remove(i);
    }
  }
  bool fits(unsigned int i) const {
    if (i < SIZE) return true;
    return overflow_.fits(i-SIZE);
  }
  void clear() {
    overflow_.clear();
    P::clear();
  }
  unsigned int get_length() const {
    return SIZE+overflow_.get_length();
  }

  void swap_with(InlineStorage<Base, SIZE> &o) {
    P::swap_with(o);
    swap(overflow_, o.overflow_);
  }
  void fill(typename Traits::PassValue v) {
    P::fill(v);
    overflow_.fill(v);
  }
};

template <class V, int S>
void swap(InlineStorage<V,S> &a,
          InlineStorage<V,S> &b) {
  a.swap_with(b);
}





IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_ATTRIBUTE_STORAGE_H */
