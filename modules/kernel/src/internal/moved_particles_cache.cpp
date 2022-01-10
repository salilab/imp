/**
 *  \file internal/moved_particles_cache.cpp
 *  \brief Cache ModelObjects that change when a Particle moves

 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <IMP/internal/moved_particles_cache.h>
#include <IMP/Model.h>
#include <IMP/ScoreState.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Cache Restraints that change when a Particle moves
const std::set<Restraint *> &
MovedParticlesRestraintCache::get_dependent_restraints(ParticleIndex pi) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    RestraintsTemp rs = IMP::get_dependent_restraints(m_, pi);
    std::set<Restraint *> rsset(rs.begin(), rs.end());
    cache_[pi] = rsset;
    it = cache_.find(pi);
  }
  return it->second;
}

const std::set<ParticleIndex> &
MovedParticlesParticleCache::get_dependent_particles(ParticleIndex pi) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    ParticlesTemp ps = IMP::get_dependent_particles(m_, pi);
    std::set<ParticleIndex> psset;
    for (ParticlesTemp::const_iterator pi = ps.begin(); pi != ps.end(); ++pi) {
      psset.insert((*pi)->get_index());
    }
    cache_[pi] = psset;
    it = cache_.find(pi);
  }
  return it->second;
}

const std::set<ScoreState *> &
MovedParticlesScoreStateCache::get_affected_score_states(ParticleIndex pi) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    ScoreStatesTemp pssin = IMP::get_dependent_score_states(m_, pi);
    // We should not be moving a particle that will just be overwritten by
    // a ScoreState, but include these anyway to make sure the Model stays
    // consistent
    ScoreStatesTemp pssout = IMP::get_required_score_states(m_, pi);
    std::set<ScoreState *> pss_set(pssin.begin(), pssin.end());
    pss_set.insert(pssout.begin(), pssout.end());
    cache_[pi] = pss_set;
    it = cache_.find(pi);
  }
  return it->second;
}

IMPKERNEL_END_INTERNAL_NAMESPACE
