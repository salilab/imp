/**
 *  \file internal/MovedParticlesCache.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H
#define IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H

#include <IMP/kernel_config.h>
#include <IMP/Model.h>
#include <IMP/dependency_graph.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

class MovedParticlesCache {
  Model *m_;
  typedef std::map<ParticleIndex, std::set<Restraint *> > CacheMap;
  CacheMap cache_;

public:
  MovedParticlesCache(Model *m) : m_(m) {}

  const std::set<Restraint *> &get_dependent_restraints(ParticleIndex pi) {
    CacheMap::const_iterator it = cache_.find(pi);
    if (it == cache_.end()) {
      RestraintsTemp rs = IMP::get_dependent_restraints(m_, pi);
      std::set<Restraint *> rsset(rs.begin(), rs.end());
      cache_[pi] = rsset;
      it = cache_.find(pi);
    }
    return it->second;
  }

  // clear when dependency graph changes, and/or
  // particles/restraints added/removed
  void clear() { cache_.clear(); }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H */
