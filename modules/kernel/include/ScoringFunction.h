/**
 *  \file IMP/kernel/ScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORING_FUNCTION_H
#define IMPKERNEL_SCORING_FUNCTION_H

#include <IMP/kernel/kernel_config.h>
#include "declare_ScoringFunction.h"
#include "declare_Model.h"
#include <boost/tuple/tuple.hpp>


IMPKERNEL_BEGIN_NAMESPACE

#ifndef IMP_DOXYGEN
inline void ScoringFunction::ensure_dependencies() {
  // work around NullScoringFunction which has no model
  if (get_model() && !get_model()->get_has_dependencies()) {
    get_model()->compute_dependencies();
  }
  IMP_INTERNAL_CHECK(!get_model() || get_model()->get_has_dependencies(),
                     "Dependencies are still bad...");
}
inline const ScoreStatesTemp& ScoringFunction::get_score_states() {
  ensure_dependencies();
  return ss_;
}
#endif

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_SCORING_FUNCTION_H */
