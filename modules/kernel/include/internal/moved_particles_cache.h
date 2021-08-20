/**
 *  \file internal/moved_particles_cache.h
 *  \brief Cache ModelObjects that change when a Particle moves

 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H
#define IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H

#include <IMP/kernel_config.h>
#include <IMP/base_types.h>
#include <IMP/dependency_graph.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Cache Restraints that change when a Particle moves
class MovedParticlesRestraintCache {
  Model *m_;
  typedef std::map<ParticleIndex, std::set<Restraint *> > CacheMap;
  CacheMap cache_;

public:
  MovedParticlesRestraintCache(Model *m) : m_(m) {}

  //! Get a set of all restraints that take this particle as input
  /** The dependency graph is used to properly account for indirect
      relationships between a particle and a restraint (e.g. via a ScoreState).
      The result is cached; the cache should be cleared whenever the
      dependency graph changes.
    */
  const std::set<Restraint *> &get_dependent_restraints(ParticleIndex pi);

  // clear when dependency graph changes, and/or
  // particles/restraints added/removed
  void clear() { cache_.clear(); }
};

//! Cache ScoreStates that change when a Particle moves
class MovedParticlesScoreStateCache {
  Model *m_;
  typedef std::map<ParticleIndex, ScoreStatesTemp> CacheMap;
  CacheMap cache_;

public:
  MovedParticlesScoreStateCache(Model *m) : m_(m) {}

  //! Get a list of all ScoreStates affected by this particle
  /** The list is a subset of ScoreStates that the given ScoringFunction
      requires, that either take the particle as input/output or are marked
      as always requiring evaluation (e.g. for logging).
    */
  const ScoreStatesTemp &get_affected_score_states(ParticleIndex pi,
                                                   ScoringFunction *sf);

  // clear when dependency graph changes, and/or
  // particles/scorestates added/removed
  void clear() { cache_.clear(); }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H */
