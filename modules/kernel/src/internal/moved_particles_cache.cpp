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
    RestraintsTemp rs = m_->get_dependent_restraints_uncached(pi);
    std::set<Restraint *> rsset(rs.begin(), rs.end());
    cache_[pi] = rsset;
    it = cache_.find(pi);
  }
  return it->second;
}

const ParticleIndexes &
MovedParticlesParticleCache::get_dependent_particles(ParticleIndex pi) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    ParticlesTemp ps = m_->get_dependent_particles_uncached(pi);
    ParticleIndexes pis;
    for (ParticlesTemp::const_iterator pit = ps.begin();
         pit != ps.end(); ++pit) {
      pis.push_back((*pit)->get_index());
    }
    cache_[pi] = pis;
    it = cache_.find(pi);
  }
  return it->second;
}

const std::set<ScoreState *> &
MovedParticlesScoreStateCache::get_affected_score_states(ParticleIndex pi) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    ScoreStatesTemp pssin = m_->get_dependent_score_states_uncached(pi);
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
