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
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/incremental_scoring_function.h>
#include <numeric>
IMPCORE_BEGIN_NAMESPACE
/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/


IncrementalScoringFunction
::IncrementalScoringFunction(const ParticlesTemp &ps,
                             const RestraintsTemp &rs,
                             double weight, double max,
                             std::string name):
  ScoringFunction(rs[0]->get_model(),
                  name), weight_(weight), max_(max) {
  IMP_OBJECT_LOG;
  initialized_=false;
  all_= IMP::internal::get_index(ps);
  Pointer<ScoringFunction> suppress_error(this);
  create_flattened_restraints(rs);
  suppress_error.release();
}

void IncrementalScoringFunction
::initialize() {
  IMP_OBJECT_LOG;
  create_scoring_functions();
  // suppress check error
  initialize_scores();
  moved_=base::get_invalid_index<ParticleIndexTag>();
  initialized_=true;
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i]->initialize();
  }
}


void IncrementalScoringFunction::create_scoring_functions() {
  IMP_OBJECT_LOG;
  if (flattened_restraints_.empty()) return;
  compatibility::map<Restraint*,int> all_set;
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    all_set[flattened_restraints_[i]]=i;
  }
  DependencyGraph dg
    = get_dependency_graph(flattened_restraints_[0]->get_model());

  for (unsigned int i=0; i< all_.size(); ++i) {
    Particle *p= get_model()->get_particle(all_[i]);
    RestraintsTemp cr= get_dependent_restraints(p, ParticlesTemp(),
                                                dg);
    RestraintsTemp mr;
    Ints mi;
    for (unsigned int j=0; j < cr.size(); ++j) {
      if (all_set.find(cr[j]) != all_set.end()) {
        mr.push_back(cr[j]);
        mi.push_back(all_set.find(cr[j])->second);
      }
    }
    IMP_LOG(TERSE, "Particle " << Showable(p) << " has restraints "
            << mr << std::endl);
    if (!mr.empty()) {
      scoring_functions_[all_[i]]
        = new internal::SingleParticleScoringFunction(all_[i],
                                                      mr, mi);
      scoring_functions_[all_[i]]->set_name(p->get_name()
                                            + " restraints");
    }
  }

  for (unsigned int i=0; i< nbl_.size(); ++i) {
    Pointer<Restraint> pr=nbl_[i]->get_dummy_restraint();
    // avoid any complex restraints
    Pointer<Restraint> pdr= pr->create_decomposition();
    for (ScoringFunctionsMap::iterator it= scoring_functions_.begin();
         it != scoring_functions_.end(); ++it) {
      it->second->add_dummy_restraint(pdr);
    }
  }
}
void IncrementalScoringFunction::initialize_scores() {
  IMP_OBJECT_LOG;
  IMP_NEW(RestraintsScoringFunction, sf, (flattened_restraints_));
  sf->evaluate(false);
  flattened_restraints_scores_.resize(flattened_restraints_.size());
  for (unsigned int i=0; i < flattened_restraints_.size(); ++i) {
    flattened_restraints_scores_[i]
      = flattened_restraints_[i]->get_last_score();
  }
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i]->initialize();
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
  // restraint sets get lost and cause warnings. Not sure how to handle them.
  flattened_restraints_=IMP::get_restraints(decomposed.begin(),
                                                    decomposed.end());

}
void IncrementalScoringFunction::reset() {
  initialize_scores();
}
void IncrementalScoringFunction::reset_moved_particles() {
  moved_=base::get_invalid_index<ParticleIndexTag>();
  rollback();
}
void IncrementalScoringFunction::set_moved_particles(const ParticlesTemp &p) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(p.size()<=1, "Can only move one particle at a time");
  IMP_USAGE_CHECK(p.empty() || scoring_functions_.find(p[0]->get_index())
                  != scoring_functions_.end(),
                  "Particle " << Showable(p[0]) << " was not in the list of "
                  << "particles passed to the constructor.");
  if (! initialized_) {
    initialize();
  }
  if (p.empty()) {
    moved_=base::get_invalid_index<ParticleIndexTag>();
    for (unsigned int i=0; i< nbl_.size(); ++i) {
      nbl_[i]->set_moved(ParticleIndexes());
    }
  } else {
    moved_=p[0]->get_index();
    for (unsigned int i=0; i< nbl_.size(); ++i) {
      nbl_[i]->set_moved(ParticleIndexes(1, p[0]->get_index()));
    }
  }
}
// make sure to reset last scores
void IncrementalScoringFunction::rollback() {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
    int index=old_incremental_score_indexes_[i];
    IMP_LOG(TERSE, "Rolling back score for "
            << Showable(flattened_restraints_[index]) << " from "
                << flattened_restraints_scores_[index]
                << " to " << old_incremental_scores_[i] << std::endl);

    flattened_restraints_scores_[index]
      = old_incremental_scores_[i];
  }
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i]->rollback();
  }
}
void IncrementalScoringFunction::add_close_pair_score(PairScore *ps,
                                                      double distance,
                                                      const ParticlesTemp
                                                      &particles,
                                                      const PairFilters &
                                                      filters) {
  IMP_OBJECT_LOG;
  nbl_.push_back(new internal::NBLScoring(ps, distance,all_, particles,
                                        filters, weight_, max_));
  // This ensures that the score states needed for the non-bonded terms
  // are updated.
  Pointer<Restraint> pr=nbl_.back()->get_dummy_restraint();
  // avoid any complex restraints
  Pointer<Restraint> pdr= pr->create_decomposition();
  for (ScoringFunctionsMap::iterator it= scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    it->second->add_dummy_restraint(pdr);
  }
  // so that the score states for the ScoringFunctions are recomputed
  get_model()->reset_dependencies();
}

void IncrementalScoringFunction::clear_close_pair_scores() {
  for (ScoringFunctionsMap::iterator it= scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    it->second->clear_dummy_restraints();
  }
  nbl_.clear();
}


ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_good(bool ,
                                    const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate(bool derivatives,
                                         const ScoreStatesTemp &ss) {
  IMP_OBJECT_LOG;
  if (! initialized_) {
    initialize();
  }
  IMP_USAGE_CHECK(ss.empty(), "Where did the score states come from?");
  old_incremental_scores_.clear();
  old_incremental_score_indexes_.clear();
  //IMP_LOG(TERSE, "Evaluate with " << moved_ << std::endl);
  if (moved_ != base::get_invalid_index<ParticleIndexTag>()) {
    ScoringFunctionsMap::const_iterator it=scoring_functions_.find(moved_);
    if (it != scoring_functions_.end()) {
      it->second->evaluate(derivatives);
      old_incremental_score_indexes_=it->second->get_restraint_indexes();
      old_incremental_scores_.resize(old_incremental_score_indexes_.size());
      for (unsigned int i=0; i< old_incremental_score_indexes_.size(); ++i) {
        int index=old_incremental_score_indexes_[i];
        old_incremental_scores_[i]=flattened_restraints_scores_[index];
        double score=flattened_restraints_[index]->get_last_score();
        IMP_LOG(TERSE, "Updating score for "
                << Showable(flattened_restraints_[index]) << " from "
                << old_incremental_scores_[i]
                << " to " << score << std::endl);
        flattened_restraints_scores_[index]=score;
      }
    }
  }
  IMP_LOG(TERSE, "Scores are " << flattened_restraints_scores_ << std::endl);
  double score=std::accumulate(flattened_restraints_scores_.begin(),
                               flattened_restraints_scores_.end(),
                               0.0)*weight_;
  // do nbl stuff
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    double cscore= nbl_[i]->get_score();
    IMP_LOG(TERSE, "NBL score is " << cscore << std::endl);
    score+=cscore;
  }
  return std::make_pair(score, true);
}
ScoringFunction::ScoreIsGoodPair
IncrementalScoringFunction::do_evaluate_if_below(bool ,
                                                 double ,
                                                 const ScoreStatesTemp &) {
  IMP_NOT_IMPLEMENTED;
}
Restraints IncrementalScoringFunction::create_restraints() const {
  Restraints ret;
  for (ScoringFunctionsMap::const_iterator it= scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    ret+= it->second->create_restraints();
  }
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    ret.push_back(nbl_[i]->create_restraint());
  }
  return ret;
}


void IncrementalScoringFunction::do_show(std::ostream &) const {
}
IncrementalScoringFunction::Wrapper::~Wrapper(){
  for (unsigned int i=0; i< size(); ++i) {
    delete operator[](i);
  }
}


//! all real work is passed off to other ScoringFunctions
ScoreStatesTemp IncrementalScoringFunction
::get_required_score_states(const DependencyGraph &) const {
    return ScoreStatesTemp();
  }
IMPCORE_END_NAMESPACE
