/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/IncrementalScoringFunction.h>
#include <IMP/RestraintSet.h>
#include <IMP/Restraint.h>
#include <IMP/dependency_graph.h>
#include <IMP/compatibility/set.h>
#include <numeric>
IMPCORE_BEGIN_NAMESPACE
/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/


IncrementalScoringFunction::SingleParticleScoringFunction
::SingleParticleScoringFunction(ParticleIndex pi,
                                const RestraintsTemp &rs,
                                const Ints &indexes):
  RestraintsScoringFunction(rs),
  indexes_(indexes), pi_(pi) {}

const ScoreStatesTemp
IncrementalScoringFunction::SingleParticleScoringFunction
::get_extra_score_states(const DependencyGraph &dg) const {
  return IMP::get_dependent_score_states(get_model()->get_particle(pi_),
                                         ParticlesTemp(),
                                         dg);
}

IncrementalScoringFunction
::IncrementalScoringFunction(const RestraintsTemp &rs):
  ScoringFunction(rs[0]->get_model(),
                  "IncrementalScoringFunction%1%") {
  create_flattened_restraints(rs);
  create_scoring_functions();
  initialize_scores();
}
void IncrementalScoringFunction::create_scoring_functions() {
  if (flattened_restraints_.empty()) return;
  compatibility::map<Restraint*,int> all_set;
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    all_set[flattened_restraints_[i]]=i;
  }
  DependencyGraph dg
    = get_dependency_graph(flattened_restraints_[0]->get_model());
  for (Model::ParticleIterator it=get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
    RestraintsTemp cr= get_dependent_restraints(*it, ParticlesTemp(),
                                                dg);
    RestraintsTemp mr;
    Ints mi;
    for (unsigned int i=0; i < cr.size(); ++i) {
      if (all_set.find(cr[i]) != all_set.end()) {
        mr.push_back(all_set.find(cr[i])->first);
        mi.push_back(all_set.find(cr[i])->second);
      }
    }
    if (!mr.empty()) {
      scoring_functions_[(*it)->get_index()]
        = new SingleParticleScoringFunction((*it)->get_index(),
                                            mr, mi);
    }
  }
}
void IncrementalScoringFunction::initialize_scores() {
  IMP_NEW(RestraintsScoringFunction, sf, (flattened_restraints_));
  sf->evaluate(false);
  flattened_restraints_scores_.resize(flattened_restraints_.size());
  for (unsigned int i=0; i < flattened_restraints_.size(); ++i) {
    flattened_restraints_scores_[i]
      = flattened_restraints_[i]->get_last_score();
  }
}
void IncrementalScoringFunction
::create_flattened_restraints(const RestraintsTemp &rs) {
  Restraints decomposed;
  for (unsigned int i=0; i < rs.size(); ++i) {
    Pointer<Restraint> cur= rs[i]->create_decomposition();
    if (cur) {
      decomposed.push_back(cur);
    }
  }
  flattened_restraints_=IMP::get_restraints(decomposed.begin(),
                                                    decomposed.end());

}

void IncrementalScoringFunction::set_moved_particles(unsigned int move_index,
                                                     const ParticlesTemp &p) {
  IMP_USAGE_CHECK(p.size()<=1, "Can only move one particle at a time");
  IMP_USAGE_CHECK(std::abs<int>(move_index-move_index_)==1,
                  "can only change move index by 1");
  if (move_index> move_index_) {
    if (p.empty()) {
      moved_=base::get_invalid_index<ParticleIndexTag>();
    }
    else moved_=p[0]->get_index();
  } else {
    moved_=base::get_invalid_index<ParticleIndexTag>();
    rollback();
  }
  move_index_=move_index;
}
// make sure to reset last scores
void IncrementalScoringFunction::rollback() {
  for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
    flattened_restraints_scores_[old_incremental_score_indexes_[i]]
      = old_incremental_scores_[i];
  }
  // rollback nbl
}
unsigned int IncrementalScoringFunction::get_move_index() const {
  return move_index_;
}
void IncrementalScoringFunction::add_close_pair_score(PairScore *ps,
                                                      double distance,
                                                      const ParticlesTemp
                                                      &particles,
                                                      const PairFilters &
                                                      filters) {
  IMP_NOT_IMPLEMENTED;
}



ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_good(bool ,
                                    const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate(bool derivatives,
                                         const ScoreStatesTemp &ss) {
  IMP_USAGE_CHECK(ss.empty(), "Where did the score states come from?");
  old_incremental_scores_.clear();
  old_incremental_score_indexes_.clear();
  if (moved_ != base::get_invalid_index<ParticleIndexTag>()) {
    ScoringFunctionsMap::const_iterator it=scoring_functions_.find(moved_);
    if (it != scoring_functions_.end()) {
      it->second->evaluate(derivatives);
      old_incremental_score_indexes_=it->second->get_restraint_indexes();
      for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
        double score=flattened_restraints_[old_incremental_score_indexes_[i]]
          ->get_last_score();
        old_incremental_scores_.push_back(score);
      }
    }
  }
  // do nbl stuff
  return std::make_pair(std::accumulate(flattened_restraints_scores_.begin(),
                                        flattened_restraints_scores_.end(),
                                        0.0), true);
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_below(bool ,
                                                 double ,
                                                 const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
RestraintsTemp IncrementalScoringFunction::get_restraints() const {
  return RestraintsTemp();
}


void IncrementalScoringFunction::do_show(std::ostream &) const {
}

IMPCORE_END_NAMESPACE
