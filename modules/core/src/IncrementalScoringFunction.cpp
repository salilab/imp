/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/IncrementalScoringFunction.h>
#include <IMP/RestraintSet.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/dependency_graph.h>
#include <IMP/base/set.h>
#include <IMP/core/XYZ.h>
#include <IMP/kernel/internal/container_helpers.h>
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
IMP::kernel::Model *extract_model(const kernel::ParticlesTemp &ps) {
  IMP_USAGE_CHECK(ps.size() > 0,
                  "needs at least one particle to extract a model");
  return ps[0]->get_model();
}
}

IncrementalScoringFunction::IncrementalScoringFunction(
    const kernel::ParticlesTemp &ps, const kernel::RestraintsTemp &rs,
    double weight, double max, std::string name)
    : ScoringFunction(extract_model(ps), name), weight_(weight), max_(max) {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Creating IncrementalScoringFunction with particles "
                << ps << " and restraints " << rs << std::endl);
  all_ = IMP::internal::get_index(ps);
  base::Pointer<ScoringFunction> suppress_error(this);
  create_flattened_restraints(rs);
  create_scoring_functions();
  dirty_ = all_;
  flattened_restraints_scores_.resize(flattened_restraints_.size());
  suppress_error.release();
}

namespace {
class IncrementalRestraintsScoringFunction
    : public IMP::internal::RestraintsScoringFunction {
 public:
  IncrementalRestraintsScoringFunction(
      const kernel::RestraintsTemp &rs, double weight = 1.0,
      double max = NO_MAX,
      std::string name = "IncrementalRestraintsScoringFunction%1%")
      : IMP::internal::RestraintsScoringFunction(rs, weight, max, name) {}
  // don't depend on optimized particles
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return get_restraints();
  }
};
}

IncrementalScoringFunction::Data IncrementalScoringFunction::create_data(
    kernel::ParticleIndex pi, const base::map<Restraint *, int> &all,
    const kernel::Restraints &dummies) const {
  kernel::RestraintsTemp cr = get_dependent_restraints(get_model(), pi);
  IMP_LOG_TERSE("Dependent restraints for particle "
                << get_model()->get_particle_name(pi) << " are " << cr
                << std::endl);
  Data ret;
  for (unsigned int j = 0; j < cr.size(); ++j) {
    if (all.find(cr[j]) != all.end()) {
      int index = all.find(cr[j])->second;
      IMP_INTERNAL_CHECK(
          std::find(ret.indexes.begin(), ret.indexes.end(), index) ==
              ret.indexes.end(),
          "Found duplicate restraint " << Showable(cr[j]) << " in list " << cr);
      ret.indexes.push_back(index);
    }
  }
  cr += kernel::RestraintsTemp(dummies.begin(), dummies.end());
  ret.sf = new IncrementalRestraintsScoringFunction(
      cr, 1.0, NO_MAX, get_model()->get_particle_name(pi) + " restraints");
  return ret;
}

void IncrementalScoringFunction::handle_set_has_required_score_states(bool tf) {
  IMP_OBJECT_LOG;
  if (tf) {
    create_scoring_functions();
  } else {
  }
}

void IncrementalScoringFunction::create_scoring_functions() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Creating scoring functions" << std::endl);
  if (flattened_restraints_.empty()) return;

  base::map<kernel::Restraint *, int> mp;
  IMP_LOG_TERSE("All restraints are " << flattened_restraints_ << std::endl);
  for (unsigned int i = 0; i < flattened_restraints_.size(); ++i) {
    mp[flattened_restraints_[i]] = i;
  }

  kernel::Restraints drs;
  for (unsigned int i = 0; i < nbl_.size(); ++i) {
    // This ensures that the score states needed for the non-bonded terms
    drs.push_back(nbl_[i]->get_dummy_restraint());
  }

  for (unsigned int i = 0; i < all_.size(); ++i) {
    scoring_functions_[all_[i]] = create_data(all_[i], mp, drs);
  }
}

void IncrementalScoringFunction::create_flattened_restraints(
    const kernel::RestraintsTemp &rs) {
  kernel::Restraints decomposed;
  for (unsigned int i = 0; i < rs.size(); ++i) {
    base::Pointer<kernel::Restraint> cur = rs[i]->create_decomposition();
    if (cur) {
      decomposed.push_back(cur);
      cur->set_was_used(true);  // suppress message about the score states
    }
  }
  // restraint sets get lost and cause warnings. Not sure how to handle them.
  flattened_restraints_ =
      IMP::get_restraints(decomposed.begin(), decomposed.end());
  IMP_LOG_TERSE("Flattened restraints are " << flattened_restraints_
                                            << std::endl);
}
void IncrementalScoringFunction::reset_moved_particles() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Resetting moved particles" << std::endl);
  set_moved_particles(last_move_);
  last_move_.clear();
}
void IncrementalScoringFunction::set_moved_particles(
    const kernel::ParticleIndexes &p) {
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i = 0; i < p.size(); ++i) {
      IMP_USAGE_CHECK(std::find(all_.begin(), all_.end(), p[i]) != all_.end(),
                      "Particle " << Showable(p[i])
                                  << " was not in the list of "
                                  << "particles passed to the constructor.");
    }
  }
  last_move_ = p;
  for (unsigned int i = 0; i < nbl_.size(); ++i) {
    nbl_[i]->set_moved(last_move_);
  }
  dirty_ += last_move_;
}

void IncrementalScoringFunction::add_close_pair_score(
    PairScore *ps, double distance, const kernel::ParticlesTemp &particles) {
  add_close_pair_score(ps, distance, particles, PairPredicates());
}

void IncrementalScoringFunction::add_close_pair_score(
    PairScore *ps, double distance, const kernel::ParticlesTemp &particles,
    const PairPredicates &filters) {
  IMP_OBJECT_LOG;
  for (unsigned int i = 0; i < filters.size(); ++i) {
    filters[i]->set_was_used(true);
  }
  nbl_.push_back(new internal::NBLScoring(ps, distance, all_, particles,
                                          filters, weight_, max_));
  set_has_dependencies(false);
}

void IncrementalScoringFunction::clear_close_pair_scores() {
  nbl_.clear();
  set_has_dependencies(false);
}
ParticleIndexes IncrementalScoringFunction::get_movable_indexes() const {
  return all_;
}

void IncrementalScoringFunction::do_non_incremental_evaluate() {
  if (!non_incremental_) {
    non_incremental_ = IMP::ScoringFunctionAdaptor(flattened_restraints_);
    non_incremental_->set_name(get_name() + "-all");
  }
  non_incremental_->evaluate(false);
  for (unsigned int i = 0; i < flattened_restraints_.size(); ++i) {
    flattened_restraints_scores_[i] =
        flattened_restraints_[i]->get_last_score();
  }
  dirty_.clear();
}

void IncrementalScoringFunction::do_add_score_and_derivatives(
    ScoreAccumulator sa, const ScoreStatesTemp &) {
  IMP_OBJECT_LOG;
  // ignore score states as we handle them internally
  if (dirty_.size() > all_.size() * .1) {
    IMP_LOG_TERSE("Doing non-incremental evaluate" << std::endl);
    do_non_incremental_evaluate();
  } else {
    IMP_LOG_TERSE("Doing incremental evaluate: " << dirty_ << std::endl);
    for (unsigned int i = 0; i < dirty_.size(); ++i) {
      ScoringFunctionsMap::const_iterator it =
          scoring_functions_.find(dirty_[i]);
      if (it != scoring_functions_.end()) {
        it->second.sf->evaluate(sa.get_derivative_accumulator());
        for (unsigned int i = 0; i < it->second.indexes.size(); ++i) {
          int index = it->second.indexes[i];
          double score = flattened_restraints_[index]->get_last_score();
          IMP_LOG_TERSE("Updating score for "
                        << Showable(flattened_restraints_[index]) << " to "
                        << score << std::endl);
          flattened_restraints_scores_[index] = score;
          IMP_INTERNAL_CHECK_FLOAT_EQUAL(
              score,
              flattened_restraints_[index]->unprotected_evaluate(nullptr), .1);
        }
      }
    }
  }
  IMP_LOG_TERSE("Scores are " << flattened_restraints_scores_ << std::endl);
  double score = std::accumulate(flattened_restraints_scores_.begin(),
                                 flattened_restraints_scores_.end(), 0.0) *
                 weight_;
  // non-incremental ignores nbl terms
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (non_incremental_) {
      double niscore = non_incremental_->evaluate(false);
      IMP_UNUSED(niscore);
      if (std::abs(niscore - score) > .01 * (niscore + score)) {
        IMP_WARN("Scores: " << niscore << " vs " << score << std::endl);
        IMP_WARN("Dirty: " << dirty_ << " " << scoring_functions_.size()
                           << std::endl);
        for (unsigned int i = 0; i < dirty_.size(); ++i) {
          ScoringFunctionsMap::const_iterator it =
              scoring_functions_.find(dirty_[i]);
          if (it != scoring_functions_.end()) {
            IMP_WARN("Scoring function for " << dirty_[i] << " is "
                                             << it->second.sf->get_name()
                                             << std::endl);
          }
        }
        Floats before = flattened_restraints_scores_;
        do_non_incremental_evaluate();
        for (unsigned int i = 0; i < before.size(); ++i) {
          IMP_WARN(flattened_restraints_[i]->get_name()
                   << ": " << before[i] << " vs "
                   << flattened_restraints_scores_[i] << std::endl);
        }
        IMP_INTERNAL_CHECK(
            false, "Incremental and non-incremental scores don't match");
      }
    }
  }
  // do nbl stuff
  for (unsigned int i = 0; i < nbl_.size(); ++i) {
    double cscore = nbl_[i]->get_score();
    IMP_LOG_TERSE("NBL score is " << cscore << std::endl);
    score += cscore;
  }
  sa.add_score(score);
  dirty_.clear();
}

Restraints IncrementalScoringFunction::create_restraints() const {
  kernel::Restraints ret;
  for (ScoringFunctionsMap::const_iterator it = scoring_functions_.begin();
       it != scoring_functions_.end(); ++it) {
    ret += it->second.sf->create_restraints();
  }
  for (unsigned int i = 0; i < nbl_.size(); ++i) {
    ret.push_back(nbl_[i]->create_restraint());
  }
  return ret;
}

IncrementalScoringFunction::Wrapper::~Wrapper() {
  for (unsigned int i = 0; i < size(); ++i) {
    delete operator[](i);
  }
}

//! all real work is passed off to other ScoringFunctions
ModelObjectsTemp IncrementalScoringFunction::do_get_inputs() const {
  return kernel::ModelObjectsTemp();
}

IncrementalScoringFunction::ScoringFunctionsMap::~ScoringFunctionsMap() {
  // move it to a temp so a second attempt to destoy it succeeds
  base::map<kernel::ParticleIndex, Data> t;
  std::swap<base::map<kernel::ParticleIndex, Data> >(*this, t);
}
IMPCORE_END_NAMESPACE
