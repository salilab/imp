/**
 *  \file ArrayOnAttributesHelper.h
 *  \brief Various methods for managing an array of attributes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_ARRAY_ON_ATTRIBUTES_HELPER_H
#define IMPCORE_ARRAY_ON_ATTRIBUTES_HELPER_H

#include "../config.h"
#include "../macros.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>

#include <string>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

template <class Key, class Value>
struct ArrayData: public RefCountedObject {
  ArrayData(std::string p):
    num_key((p+"_number").c_str()),
    prefix(p) {
  }
  ~ArrayData(){}
  std::vector<Key> keys;
  const IntKey num_key;
  const std::string prefix;
};

template <class K, class V>
std::ostream &operator<<(std::ostream &out, const ArrayData<K,V> &d) {
  out << d.prefix;
  return out;
}


template <class KeyT, class ValueT>
struct ArrayOnAttributesHelper {
  typedef KeyT Key;
  typedef ValueT Value;
  typedef ArrayData<Key, Value> Data;

  ArrayOnAttributesHelper(){}
  ArrayOnAttributesHelper(std::string p): data_(new Data(p)){}



  unsigned int get_size(const Particle *p) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    return p->get_value(data_->num_key);
  }

  void initialize_particle(Particle *p) {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    p->add_attribute(data_->num_key, 0);
  }

  Value get_value(const Particle *p, unsigned int i) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    IMP_check(static_cast<unsigned int>(p->get_value(data_->num_key)) > i,
              "Out of range attribute in array",
              IndexException);
    return p->get_value(get_key(i));
  }

  void set_value(Particle *p,
                 unsigned int i,
                 Value v) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    IMP_check(data_->keys.size() > i, "Out of range attribute in array",
              IndexException);
    IMP_check(p->get_value(data_->num_key) > i,
              "Out of range attribute in array",
              IndexException);
    p->set_value(data_->keys[i], v);
  }

  unsigned int push_back(Particle *p,
                         Value v) {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    unsigned int osz= p->get_value(data_->num_key);
    Key k= get_key(osz);
    p->add_attribute(k, v);
    p->set_value(data_->num_key, osz+1);
    return osz;
  }

  void insert(Particle *p,
              unsigned int loc,
              Value v) {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    unsigned int osz= p->get_value(data_->num_key);
    IMP_check(loc <= osz, "Attribute array must be contiguous",
              IndexException);
    for (unsigned int i=loc; i < osz; ++i) {
      Key k= get_key(i);
      Value t= p->get_value(k);
      p->set_value(k, v);
      v=t;
    }
    Key k= get_key(osz);
    p->add_attribute(k, v);
    p->set_value(data_->num_key, osz+1);
  }


  void erase(Particle *p,
             unsigned int loc) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    unsigned int osz= p->get_value(data_->num_key);
    IMP_check(loc <= osz, "Can only erase values in array",
              IndexException);
    for (unsigned int i=loc+1; i < osz; ++i) {
      Key k= data_->keys[i];
      Key kl= data_->keys[i-1];
      p->set_value(kl, p->get_value(k));
    }
    Key k= data_->keys[osz-1];
    p->remove_attribute(k);
    p->set_value(data_->num_key, osz-1);
  }

  std::string get_prefix() const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    return data_->prefix;
  }


  bool has_required_attributes(Particle *p) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    return p->has_attribute(data_->num_key);
  }

  void add_required_attributes(Particle *p) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    p->add_attribute(data_->num_key, 0);
  }

private:
  Key get_key(unsigned int i) const {
    IMP_assert(data_, "Cannot used uninitialized HierarchyTraits");
    while (!(i < data_->keys.size())) {
      std::ostringstream oss;
      oss << data_->prefix << data_->keys.size();
      data_->keys.push_back(Key(oss.str().c_str()));
    }
    return data_->keys[i];
  }

  mutable Pointer<Data> data_;
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_ARRAY_ON_ATTRIBUTES_HELPER_H */
