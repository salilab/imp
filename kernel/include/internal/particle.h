/**
 *  \file internal/particle.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_PARTICLE_H
#define IMP_INTERNAL_PARTICLE_H

#include "../config.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE


template <class Key, class Particle>
class IsAttribute
{
  const Particle *map_;
public:
  IsAttribute(): map_(NULL){}
  IsAttribute(const Particle *map): map_(map) {}
  bool operator()(Key k) const {
    return map_->has_attribute(k);
  }
};

template <class Key, class Particle>
class IsOptimized
{
  const Particle *map_;
public:
  IsOptimized(): map_(NULL){}
  IsOptimized(const Particle *map): map_(map) {}
  bool operator()(Key k) const {
    return map_->get_is_optimized(k);
  }
};

template <class Key, class Particle, class Check>
struct ParticleKeyIterator {
  typedef boost::counting_iterator<Key, boost::forward_traversal_tag,
                                   unsigned int> KeyIterator;
  typedef boost::filter_iterator<Check, KeyIterator> Iterator;

  static Iterator create_iterator(const Particle *p, unsigned int cur,
                                  unsigned int len) {
    return Iterator(Check(p),
                    KeyIterator(Key(cur)),
                    KeyIterator(Key(len)));
  }
  static std::vector<Key> get_keys(const Particle *p, unsigned int len) {
    std::vector<Key> ret(create_iterator(p, 0, len),
                         create_iterator(p, len, len));
    return ret;
  }
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_PARTICLE_H */
