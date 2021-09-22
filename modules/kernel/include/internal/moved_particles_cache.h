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
#include <IMP/ScoreState.h>

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

//! Cache Particles that change when a Particle moves
class MovedParticlesParticleCache {
  Model *m_;
  typedef std::map<ParticleIndex, std::set<ParticleIndex> > CacheMap;
  CacheMap cache_;

public:
  MovedParticlesParticleCache(Model *m) : m_(m) {}

  //! Get a set of all particles that move when this particle moves
  /** The result is cached; the cache should be cleared whenever the
      dependency graph changes.
    */
  const std::set<ParticleIndex> &get_dependent_particles(ParticleIndex pi);

  // clear when dependency graph changes, and/or
  // particles/restraints added/removed
  void clear() { cache_.clear(); }
};

//! Cache ScoreStates that change when a Particle moves
class MovedParticlesScoreStateCache {
  Model *m_;
  typedef std::map<ParticleIndex, std::set<ScoreState *> > CacheMap;
  CacheMap cache_;

public:
  MovedParticlesScoreStateCache(Model *m) : m_(m) {}

  //! Get the set of all ScoreStates affected by this particle
  /** The set is all ScoreStates that take the particle as input/output. */
  const std::set<ScoreState *> &get_affected_score_states(ParticleIndex pi);

  // clear when dependency graph changes, and/or
  // particles/scorestates added/removed
  void clear() { cache_.clear(); }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_MOVED_PARTICLES_CACHE_H */
