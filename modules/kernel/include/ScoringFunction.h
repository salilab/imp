/**
 *  \file IMP/ScoringFunction.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORING_FUNCTION_H
#define IMPKERNEL_SCORING_FUNCTION_H

#include "kernel_config.h"
#include "declare_ScoringFunction.h"
#include "declare_Model.h"
#include <boost/tuple/tuple.hpp>


IMP_BEGIN_NAMESPACE

inline Model *ScoringFunction::get_model() const {
  return Tracked::get_tracker();
}
inline void ScoringFunction::ensure_dependencies() {
  if (!get_model()->get_has_dependencies()) {
    get_model()->compute_dependencies();
  }
}
inline const ScoreStatesTemp& ScoringFunction::get_score_states() {
  ensure_dependencies();
  return ss_;
}
inline double ScoringFunction::evaluate_if_good(bool derivatives) {
  set_was_used(true);
  ensure_dependencies();
  boost::tie(last_score_, last_was_good_)
      = do_evaluate_if_good(derivatives, ss_);
  return last_score_;
}
inline double ScoringFunction::evaluate(bool derivatives) {
  set_was_used(true);
  ensure_dependencies();
  boost::tie(last_score_, last_was_good_)
      = do_evaluate(derivatives, ss_);
  return last_score_;
}
inline double ScoringFunction::evaluate_if_below(bool derivatives, double max) {
  set_was_used(true);
  ensure_dependencies();
  boost::tie(last_score_, last_was_good_)
      = do_evaluate_if_below(derivatives, max, ss_);
  return last_score_;
}

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_SCORING_FUNCTION_H */
