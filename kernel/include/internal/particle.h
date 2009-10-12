/**
 *  \file internal/particle.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_PARTICLE_H
#define IMP_INTERNAL_PARTICLE_H

#include "../config.h"
#include "AttributeTable.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <list>

IMP_BEGIN_NAMESPACE
class Model;
IMP_END_NAMESPACE

IMP_BEGIN_INTERNAL_NAMESPACE

struct IMPEXPORT LockedParticleException{
  const Particle *p_;
  LockedParticleException(const Particle *p);
  virtual ~LockedParticleException() throw();
};

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


struct ReadLock;

struct IMPEXPORT ParticleStorage {
  ParticleStorage(): shadow_(NULL), dirty_(false){
#if IMP_BUILD < IMP_FAST
    read_locked_=false;
#endif
  }
  ~ParticleStorage();
  typedef std::list<Particle*> Storage;
  typedef  internal::OffsetStorage<
    internal::ArrayStorage<internal::FloatAttributeTableTraits>, IMP_NUM_INLINE>
    FloatTable;
  typedef internal::ArrayStorage<internal::BoolAttributeTableTraits>
    OptimizedTable;
  typedef internal::ArrayStorage<internal::IntAttributeTableTraits>
    IntTable;
  typedef internal::ArrayStorage<internal::StringAttributeTableTraits>
    StringTable;
  typedef internal::RefCountedStorage<internal::ParticlesAttributeTableTraits>
    ParticleTable;
  typedef internal::RefCountedStorage<internal::ObjectsAttributeTableTraits>
    ObjectTable;
  typedef internal::ArrayStorage<internal::DoubleAttributeTableTraits>
    DerivativeTable;

  WeakPointer<Model> model_;

  FloatTable floats_;
  DerivativeTable derivatives_;
  OptimizedTable optimizeds_;
  IntTable ints_;
  StringTable  strings_;
  ParticleTable particles_;
  ObjectTable objects_;

  Storage::iterator iterator_;
  // manually ref counted since Pointer requires the full definition
  Particle* shadow_;

  // incremental updates
  bool dirty_;

#if IMP_BUILD < IMP_FAST
  // for testing get_used_particles()
  bool read_locked_;
#endif
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_PARTICLE_H */
