/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/IncrementalScoringFunction.h>
#include <IMP/RestraintSet.h>
#include <IMP/Restraint.h>
#include <IMP/dependency_graph.h>
#include <IMP/base/set.h>
#include <IMP/core/XYZ.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/incremental_scoring_function.h>
#include <IMP/base/check_macros.h>
#include <numeric>
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE
/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/

namespace {
  // TODO: this can be made a general library function at some point
  IMP::Model* extract_model(const ParticlesTemp &ps)
  {
    IMP_USAGE_CHECK(ps.size() > 0,
                    "needs at least one particle to extract a model");
    return ps[0]->get_model();
  }
}


IncrementalScoringFunction
::IncrementalScoringFunction(const ParticlesTemp &ps,
                             const RestraintsTemp &rs,
                             double weight, double max,
                             std::string name):
  ScoringFunction(extract_model(ps), name),
    weight_(weight),
    max_(max) {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Creating IncrementalScoringFunction with particles "
          << ps << " and restraints " << rs << std::endl);
  all_= IMP::internal::get_index(ps);
  Pointer<ScoringFunction> suppress_error(this);
  create_flattened_restraints(rs);
  create_scoring_functions();
  dirty_=all_;
  flattened_restraints_scores_.resize(flattened_restraints_.size());
  suppress_error.release();
}



void IncrementalScoringFunction::create_scoring_functions() {
  IMP_OBJECT_LOG;
  if (flattened_restraints_.empty()) return;
  base::map<Restraint*,int> all_set;
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    all_set[flattened_restraints_[i]]=i;
  }
  IMP_USAGE_CHECK(nbl_.empty(), "Can't be close pair restraints yet");

  /*Restraints dr;
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    Pointer<Restraint> pr=nbl_[i]->get_dummy_restraint();
    // avoid any complex restraints
    Pointer<Restraint> pdr= pr->create_decomposition();
    if (pdr) {
      dr.push_back(pdr);
    }
    }*/

  for (unsigned int i=0; i< all_.size(); ++i) {
    Particle *p= get_model()->get_particle(all_[i]);
    scoring_functions_[all_[i]]
        = new internal::SingleParticleScoringFunction(p->get_index(),
                                                      flattened_restraints_,
                                                      p->get_name()
                                                      + " restraints");
  }
}

void IncrementalScoringFunction
::create_flattened_restraints(const RestraintsTemp &rs) {
  Restraints decomposed;
  for (unsigned int i=0; i < rs.size(); ++i) {
    Pointer<Restraint> cur= rs[i]->create_decomposition();
    if (cur) {
      decomposed.push_back(cur);
      cur->set_was_used(true); // suppress message about the score states
    }
  }
  // restraint sets get lost and cause warnings. Not sure how to handle them.
  flattened_restraints_=IMP::get_restraints(decomposed.begin(),
                                            decomposed.end());
  IMP_LOG_TERSE( "Flattened restraints are " << flattened_restraints_
          << std::endl);

}
void IncrementalScoringFunction::reset_moved_particles() {
  set_moved_particles( last_move_ );
  last_move_.clear();
}
void IncrementalScoringFunction::set_moved_particles(const ParticleIndexes &p) {
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< p.size(); ++i) {
      IMP_USAGE_CHECK(std::find(all_.begin(), all_.end(), p[i])
                      != all_.end(),
                      "Particle " << Showable(p[i])
                      << " was not in the list of "
                      << "particles passed to the constructor.");
    }
  }
  last_move_= p;
  for (unsigned int i=0; i< nbl_.size(); ++i) {
    nbl_[i]->set_moved(last_move_);
  }
  dirty_+=last_move_;
}

void IncrementalScoringFunction::add_close_pair_score(PairScore *ps,
                                                      double distance,
                                                      const ParticlesTemp
                                                      &particles) {
  add_close_pair_score(ps, distance, particles, PairPredicates());
}

void IncrementalScoringFunction::add_close_pair_score(PairScore *ps,
                                                      double distance,
                                                      const ParticlesTemp
                                                      &particles,
                                                      const PairPredicates &
                                                      filters) {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< filters.size(); ++i) {
    filters[i]->set_was_used(true);
  }
  nbl_.push_back(new internal::NBLScoring(ps, distance,all_, particles,
                                        filters, weight_, max_));
  // This ensures that the score states needed for the non-bonded terms
  // are updated.
  Pointer<Restraint> pdr=nbl_.back()->get_dummy_restraint();
  for (ScoringFunctionsMap::iterator it= scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    it->second->add_dummy_restraint(pdr);
  }
  // so that the score states for the ScoringFunctions are recomputed
  get_model()->clear_caches();
}

void IncrementalScoringFunction::clear_close_pair_scores() {
  for (ScoringFunctionsMap::iterator it= scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    it->second->clear_dummy_restraints();
  }
  nbl_.clear();
}
ParticleIndexes IncrementalScoringFunction::get_movable_particles() const {
  return all_;
}

void
IncrementalScoringFunction::do_non_incremental_evaluate() {
  if (!non_incremental_) {
    non_incremental_=IMP::ScoringFunctionAdaptor(flattened_restraints_);
  }
  non_incremental_->evaluate(false);
  for (unsigned int i=0; i< flattened_restraints_.size(); ++i) {
    flattened_restraints_scores_[i]= flattened_restraints_[i]->get_last_score();
  }
  dirty_.clear();
}

void
IncrementalScoringFunction::do_add_score_and_derivatives(ScoreAccumulator sa,
                                                 const ScoreStatesTemp &) {
  IMP_OBJECT_LOG;
  // ignore score states as we handle them internally
  if (dirty_.size() > all_.size()*.1) {
    IMP_LOG_TERSE("Doing non-incremental evaluate" << std::endl);
    do_non_incremental_evaluate();
  } else {
    IMP_LOG_TERSE("Doing incremental evaluate: " << dirty_ << std::endl);
    while (!dirty_.empty()) {
      ScoringFunctionsMap::const_iterator it
          =scoring_functions_.find(dirty_.back());
      dirty_.pop_back();
      if (it != scoring_functions_.end()) {
        it->second->evaluate(sa.get_derivative_accumulator());
        Ints ris=it->second->get_restraint_indexes();
        for (unsigned int i=0; i< ris.size(); ++i) {
          int index=ris[i];
          double score=flattened_restraints_[index]->get_last_score();
          IMP_LOG_TERSE( "Updating score for "
                  << Showable(flattened_restraints_[index])
                  << " to " << score << std::endl);
          flattened_restraints_scores_[index]=score;
          IMP_INTERNAL_CHECK_FLOAT_EQUAL(score,
                             flattened_restraints_[index]
                                         ->unprotected_evaluate(nullptr),
                                         .1);
        }
      }
    }
  }
  IMP_LOG_TERSE( "Scores are " << flattened_restraints_scores_ << std::endl);
  double score = std::accumulate(flattened_restraints_scores_.begin(),
                                 flattened_restraints_scores_.end(),
                                 0.0)*weight_;
  // non-incremental ignores nbl terms
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (non_incremental_) {
      double niscore= non_incremental_->evaluate(false);
      IMP_CHECK_VARIABLE(niscore);
      IMP_INTERNAL_CHECK_FLOAT_EQUAL(niscore,
                                     score,
                      "Incremental and non-incremental scores don't match: "
                                     << flattened_restraints_scores_);
    }
  }
  // do nbl stuff
  for (unsigned int i = 0; i < nbl_.size(); ++i) {
    double cscore = nbl_[i]->get_score();
    IMP_LOG_TERSE("NBL score is " << cscore << std::endl);
    score += cscore;
  }
  sa.add_score(score);
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

IncrementalScoringFunction::Wrapper::~Wrapper(){
  for (unsigned int i=0; i< size(); ++i) {
    delete operator[](i);
  }
}

//! all real work is passed off to other ScoringFunctions
ScoreStatesTemp IncrementalScoringFunction
::get_required_score_states() const {
    return ScoreStatesTemp();
  }
IMPCORE_END_NAMESPACE
