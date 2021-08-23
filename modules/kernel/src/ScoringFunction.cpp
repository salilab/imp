/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/ScoringFunction.h"
#include "IMP/Model.h"
#include "IMP/internal/evaluate_utility.h"
#include "IMP/internal/scoring_functions.h"
#include "IMP/internal/utility.h"
#include "IMP/generic.h"
#include "IMP/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

namespace {

// in namespace so it can be made a friend.
class NullScoringFunction : public ScoringFunction {
 public:
  NullScoringFunction(Model *m,
                      std::string name = "NullScoringFunction%1%")
      : ScoringFunction(m, name) {}
  void do_add_score_and_derivatives(IMP::ScoreAccumulator,
                                    const ScoreStatesTemp &) IMP_OVERRIDE {}
  Restraints create_restraints() const IMP_OVERRIDE { return Restraints(); }
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  IMP_OBJECT_METHODS(NullScoringFunction);
};

ScoringFunction *get_null_scoring_function() {
  static PointerMember<Model> m = new Model("NullModel");
  static PointerMember<ScoringFunction> sf =
      new NullScoringFunction(m, "The Null Scoring Function");
  return sf;
}
}

ScoringFunction::ScoringFunction(Model *m, std::string name)
    : ModelObject(m, name), moved_particles_cache_(m) {
  moved_particles_cache_age_ = 0;
}

double ScoringFunction::evaluate_if_good(bool derivatives) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  set_has_required_score_states(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator_if_good(derivatives);
  do_add_score_and_derivatives(sa, get_required_score_states());
  return es_.score;
}

double ScoringFunction::evaluate(bool derivatives) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  set_has_required_score_states(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator(derivatives);
  do_add_score_and_derivatives(sa, get_required_score_states());
  return es_.score;
}

const ScoreStatesTemp& ScoringFunction::get_moved_required_score_states(
                               const ParticleIndexes &moved_pis) {
  if (moved_pis.size() == 1) {
    // Clear cache if dependencies changed
    unsigned dependencies_age = get_model()->get_dependencies_updated();
    if (moved_particles_cache_age_ != dependencies_age) {
      moved_particles_cache_age_ = dependencies_age;
      moved_particles_cache_.clear();
    }
    return moved_particles_cache_.get_affected_score_states(
                                              moved_pis[0], this);
  } else {
    set_has_required_score_states(true);
    return get_required_score_states();
  }
}

double ScoringFunction::evaluate_moved(bool derivatives,
                                       const ParticleIndexes &moved_pis) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator(derivatives);
  do_add_score_and_derivatives_moved(
        sa, moved_pis, get_moved_required_score_states(moved_pis));
  return es_.score;
}

double ScoringFunction::evaluate_moved_if_below(bool derivatives,
                                       const ParticleIndexes &moved_pis,
                                       double max) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator_if_below(derivatives, max);
  do_add_score_and_derivatives_moved(
        sa, moved_pis, get_moved_required_score_states(moved_pis));
  return es_.score;
}

double ScoringFunction::evaluate_moved_if_good(bool derivatives,
                                       const ParticleIndexes &moved_pis) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator_if_good(derivatives);
  do_add_score_and_derivatives_moved(
        sa, moved_pis, get_moved_required_score_states(moved_pis));
  return es_.score;
}

double ScoringFunction::evaluate_if_below(bool derivatives, double max) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  set_has_required_score_states(true);
  es_.score = 0;
  es_.good = true;
  const ScoreAccumulator sa = get_score_accumulator_if_below(derivatives, max);
  do_add_score_and_derivatives(sa, get_required_score_states());
  return es_.score;
}

ScoringFunction *ScoringFunctionAdaptor::get(const RestraintsTemp &sf) {
  if (!sf.empty()) {
    return new internal::RestraintsScoringFunction(sf);
  } else {
    return get_null_scoring_function();
  }
}

ScoringFunction *ScoringFunctionAdaptor::get(const Restraints &sf) {
  if (!sf.empty()) {
    return new internal::RestraintsScoringFunction(sf);
  } else {
    return get_null_scoring_function();
  }
}
ScoringFunction *ScoringFunctionAdaptor::get(Restraint *sf) {
  return sf->create_scoring_function();
}

namespace {
unsigned int sf_num_children(Restraint *r) {
  RestraintSet *rs = dynamic_cast<RestraintSet *>(r);
  if (rs)
    return rs->get_number_of_restraints();
  else
    return 0;
}
}

void show_restraint_hierarchy(ScoringFunctionAdaptor r, std::ostream &out) {
  Restraints cur = r->create_restraints();
  for (unsigned int ii = 0; ii < cur.size(); ++ii) {
    Restraint *curr = cur[ii];
    curr->set_was_used(true);
    RestraintSet *rs = dynamic_cast<RestraintSet *>(curr);
    if (!rs) {
      IMP_PRINT_TREE(out, Restraint *, curr, 0,
                     dynamic_cast<RestraintSet *>(n)->get_restraint,
                     out << Showable(n) << " " << n->get_maximum_score() << " "
                         << n->get_weight());
    } else {
      IMP_PRINT_TREE(out, Restraint *, rs, sf_num_children(n),
                     dynamic_cast<RestraintSet *>(n)->get_restraint,
                     out << Showable(n) << " " << n->get_maximum_score() << " "
                         << n->get_weight());
    }
  }
}

namespace {
ScoringFunctions create_decomposition(Restraint *r, double w, double max) {
  if (!r) return ScoringFunctions();
  RestraintSet *rs = dynamic_cast<RestraintSet *>(r);
  if (rs) {
    ScoringFunctions ret;
    for (RestraintSet::RestraintIterator it = rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      ret = ret + create_decomposition(*it, w * rs->get_weight(),
                                       std::min(max, rs->get_maximum_score()));
    }
    return ret;
  } else {
    return ScoringFunctions(1, r->create_scoring_function(w, max));
  }
}

ScoringFunctions create_decomposition_into_scoring_functions(
    const RestraintsTemp &sf) {
  ScoringFunctions ret;
  for (unsigned int i = 0; i < sf.size(); ++i) {
    Pointer<Restraint> r = sf[i]->create_decomposition();
    ret = ret + create_decomposition(r, 1.0, NO_MAX);
  }
  return ret;
}
}

ScoringFunctions create_decomposition(ScoringFunction *sf) {
  ScoringFunctions ret;
  ret = create_decomposition_into_scoring_functions(sf->create_restraints());
  return ret;
}
IMPKERNEL_END_NAMESPACE
