/**
 *  \file IMP/kernel/ScoreAccumulator.h   \brief Class for adding scores from
 *                                         restraints to the model.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORE_ACCUMULATOR_H
#define IMPKERNEL_SCORE_ACCUMULATOR_H

#include <IMP/kernel/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "constants.h"
#include <IMP/base/showable_macros.h>
#include <IMP/base/value_macros.h>
#include <IMP/base/thread_macros.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/tuple_macros.h>
#include <IMP/base/math.h>
#include <IMP/base/nullptr.h>
#include <IMP/base/exception.h>

IMPKERNEL_BEGIN_NAMESPACE

class Restraint;

/** A class for storing evaluation state.*/
struct EvaluationState {
  double score;
  bool good;
  EvaluationState(double oscore, bool ogood): score(oscore), good(ogood) {}
  EvaluationState(): score(BAD_SCORE), good(false){}
  IMP_SHOWABLE_INLINE(EvaluationState, out << score << " " << good;);
};
IMP_VALUES(EvaluationState, EvaluationStates);

//! Class for adding up scores during ScoringFunction evaluation.
/** This provides a place to accumulate scores from Restraint
    evaluation. A new ScoreAccumulator is created for each Restraint
    evaluation context (eg each Restraint::add_score_and_derivatives() call).
    As a result, the ScoreAccumulator can automatically handle
    restraint and derivative weights, keeping track of maximum scores
    and other needed bookkeeping.
*/
class IMPKERNELEXPORT ScoreAccumulator: public base::Value {
  EvaluationState *score_;
  DerivativeAccumulator weight_;
  double global_max_;
  double local_max_;
  bool deriv_;
  bool abort_on_bad_;
  friend class ScoringFunction;
  friend class Restraint;
  ScoreAccumulator(EvaluationState *s, double weight, bool deriv,
                   double global_max, double local_max, bool abort_on_bad):
    score_(s),
    weight_(weight),
    global_max_(global_max),
    local_max_(local_max),
    deriv_(deriv),
    abort_on_bad_(abort_on_bad) {}


public:
  /** For swig, makes invalid (not null) state.*/
  ScoreAccumulator(): score_(nullptr) {}
  /** Compose outer accumulator with one for this restraint. */
  ScoreAccumulator(ScoreAccumulator o,
                   const Restraint *r);

  /** Compose outer accumulator with one for this restraint. */
  ScoreAccumulator(ScoreAccumulator o,
                   double weight, double local_max) {
    operator=(o);
    weight_= DerivativeAccumulator(o.weight_, weight);
    local_max_= std::min(local_max, o.local_max_);
  }

  /** Add to the total score. It will be weighted appropriately
      internally. */
  void add_score(double score) {
    double wscore= weight_.get_weight()*score;
IMP_OMP_PRAGMA(atomic)
    score_->score += wscore;
    if (score > local_max_) {
IMP_OMP_PRAGMA(critical (imp_abort))
      score_->good= false;
    }
    IMP_LOG_VERBOSE( "Score is now " << score_->score << std::endl);
  }

  //! Return if the score already exceeds the maximum
  /** Expensive restraints can check this during evaluation to determin
      if another restraint has aborted evaluation.
  */
  bool get_abort_evaluation() const {
    if (global_max_== NO_MAX && !abort_on_bad_) return false;
    if (abort_on_bad_) {
      bool good;
IMP_OMP_PRAGMA(critical (imp_abort))
      good= score_->good;
      return !good;
    } else {
      // be lazy for now
      //#pragma omp flush (score_->score)
      double score= score_->score;
      return score > global_max_;
    }
  }

  /** Return true if the current evaluation being done is one where
      scores are only consider if they are below some threshold
      (get_maximum()). */
  bool get_is_evaluate_if_below() const {return global_max_ != NO_MAX;}
  /** Return true if the current evaluation should abort if any restraint
      is above its maximum allowed good score. Restraints that take
      advantage of this in evaluation should simply consult
      get_maximum() as that will take into account both their maximum
      and that on any RestraintSets that contain them and are being
      evaluated.
  */
  bool get_is_evaluate_if_good() const {return abort_on_bad_;}
  /** The maximum allowed score for the
      Restraint::do_add_score_and_derivatives() call. */
  double get_maximum() const {return std::min(global_max_, local_max_);}

  DerivativeAccumulator *get_derivative_accumulator() {
    if (deriv_) {
      return &weight_;
    } else {
      return nullptr;
    }
  }

  IMP_SHOWABLE_INLINE(ScoreAccumulator, out << *score_);
};

IMP_VALUES(ScoreAccumulator, ScoreAccumulators);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_SCORE_ACCUMULATOR_H */
