/**
 *  \file internal/moved_particles_cache.cpp
 *  \brief Cache ModelObjects that change when a Particle moves

 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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

namespace {
class should_skip_score_state {
  const std::set<ScoreState *> &needed_ss_;
public:
  should_skip_score_state(const std::set<ScoreState *> &needed_ss)
       : needed_ss_(needed_ss) {}

  bool operator()(const ScoreState *ss) {
    return ss->get_can_skip()
        && needed_ss_.find(const_cast<ScoreState *>(ss)) == needed_ss_.end();
  }
};
}

const ScoreStatesTemp&
MovedParticlesScoreStateCache::get_affected_score_states(ParticleIndex pi,
                                                         ScoringFunction *sf) {
  CacheMap::const_iterator it = cache_.find(pi);
  if (it == cache_.end()) {
    sf->set_has_required_score_states(true);
    ScoreStatesTemp allss = sf->get_required_score_states();
    ScoreStatesTemp pssin = IMP::get_dependent_score_states(m_, pi);
    // We should not be moving a particle that will just be overwritten by
    // a ScoreState, but include these anyway to make sure the Model stays
    // consistent
    ScoreStatesTemp pssout = IMP::get_required_score_states(m_, pi);
    std::set<ScoreState *> pss_set(pssin.begin(), pssin.end());
    pss_set.insert(pssout.begin(), pssout.end());
    // Remove any ScoreState that can be skipped and that
    // doesn't affect the particle
    allss.erase(std::remove_if(allss.begin(), allss.end(),
                               should_skip_score_state(pss_set)),
                allss.end());
    cache_[pi] = allss;
    it = cache_.find(pi);
  }
  return it->second;
}

IMPKERNEL_END_INTERNAL_NAMESPACE
