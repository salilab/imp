/**
 *  \file ArrayOnAttributesHelper.h   
 *  \brief Various methods for managing an array of attributes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ARRAY_ON_ATTRIBUTES_HELPER_H
#define __IMP_ARRAY_ON_ATTRIBUTES_HELPER_H

#include "../base_types.h"

#include <string>
#include <sstream>

namespace IMP
{

namespace internal
{

template <class KeyT, class ValueT>
struct ArrayOnAttributesHelper {
  ArrayOnAttributesHelper(std::string p): prefix_(p) {}
  void initialize() {
    num_key_= IntKey((prefix_ + " number").c_str());
  }
  typedef KeyT Key;
  typedef ValueT Value;

  unsigned int get_size(const Particle *p) const {
    return p->get_value(num_key_);
  }

  void initialize_particle(Particle *p) {
    p->add_attribute(num_key_, 0);
  }

  Value get_value(const Particle *p, unsigned int i) const {
    IMP_check(keys_.size() > i, "Out of range attribute in array",
              IndexException);
    IMP_check(static_cast<unsigned int>(p->get_value(num_key_)) > i,
              "Out of range attribute in array",
              IndexException);
    return p->get_value(keys_[i]);
  }

  void set_value(Particle *p,
                 unsigned int i,
                 Value v) const {
    IMP_check(keys_.size() > i, "Out of range attribute in array",
              IndexException);
    IMP_check(p->get_value(num_key_) > i, "Out of range attribute in array",
              IndexException);
    p->set_value(keys_[i], v);
  }

  unsigned int push_back(Particle *p,
                         Value v) {
    unsigned int osz= p->get_value(num_key_);
    Key k= get_key(osz);
    p->add_attribute(k, v);
    p->set_value(num_key_, osz+1);
    return osz;
  }

  void insert(Particle *p,
              unsigned int loc,
              Value v) {
    unsigned int osz= p->get_value(num_key_);
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
    p->set_value(num_key_, osz+1);
  }


  void erase(Particle *p,
             unsigned int loc) const {
    unsigned int osz= p->get_value(num_key_);
    IMP_check(loc <= osz, "Can only erase values in array",
              IndexException);
    for (unsigned int i=loc+1; i < osz; ++i) {
      Key k= keys_[i];
      Key kl= keys_[i-1];
      p->set_value(kl, p->get_value(k));
    }
    Key k= keys_[osz-1];
    p->remove_attribute(k);
    p->set_value(num_key_, osz-1);
  }

  std::string get_prefix() const {
    return prefix_;
  }


  bool has_required_attributes(Particle *p) const {
    return p->has_attribute(num_key_);
  }

  void add_required_attributes(Particle *p) const {
    p->add_attribute(num_key_, 0);
  }

private:
  Key get_key(unsigned int i) {
    while (!(i < keys_.size())) {
      std::ostringstream oss;
      oss << prefix_ << keys_.size();
      keys_.push_back(Key(oss.str().c_str()));
    }   
    return keys_[i];
  }


  std::vector<Key> keys_;
  IntKey num_key_;
  std::string prefix_;
};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_ARRAY_ON_ATTRIBUTES_HELPER_H */
