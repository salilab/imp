/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include <IMP/log.h>
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/generic.h"
#include "IMP/ScoringFunction.h"
#include "IMP/internal/evaluate_utility.h"
#include <IMP/base/CreateLogContext.h>
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <IMP/base/internal/static.h>
#include <numeric>




IMP_BEGIN_NAMESPACE

namespace {
void check_order(const ScoreStatesTemp &ss) {
  for (unsigned int i=1; i< ss.size(); ++i) {
    IMP_USAGE_CHECK(ss[i-1]->order_ <= ss[i]->order_,
                    "Score states " << Showable(ss[i-1])
                    << " and " << Showable(ss[i])
                    << " are out of order.");
  }
}
}


void Model::before_evaluate(const ScoreStatesTemp &states) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_has_dependencies(),
                  "Model must have dependencies before calling "
                  << "before_evaluate()");
  check_order(states);
#if IMP_BUILD < IMP_FAST
  base::internal::check_live_objects();
#endif
  IMP_USAGE_CHECK(cur_stage_== internal::NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating");
  base::CreateLogContext clc("update_score_states");
  {

    internal::SFSetIt<IMP::internal::Stage>
      reset(&cur_stage_, internal::BEFORE_EVALUATING);
    boost::timer timer;
    for (unsigned int i=0; i< states.size(); ++i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      {
#if IMP_BUILD < IMP_FAST
        if (first_call_) {
          internal::SFResetBitset rbr(Masks::read_mask_, true);
          internal::SFResetBitset rbw(Masks::write_mask_, true);
          internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
          internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ModelObjects inputs=ss->get_inputs();
          ModelObjects outputs=ss->get_outputs();
          Masks::read_derivatives_mask_.reset();
          Masks::write_derivatives_mask_.reset();
          IMP_SF_SET_ONLY_2(Masks::read_mask_, inputs, outputs);
          IMP_SF_SET_ONLY(Masks::write_mask_, outputs);
          IMP_SF_SET_ONLY(Masks::add_remove_mask_, outputs);
          ss->before_evaluate();
        } else {
          ss->before_evaluate();
        }
#else
        ss->before_evaluate();
#endif
      }
      if (gather_statistics_) {
        add_to_update_before_time(ss, timer.elapsed());
      }
      //IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
}

void Model::after_evaluate(const ScoreStatesTemp &states,
                           bool calc_derivs) {
  IMP_OBJECT_LOG;
  check_order(states);
  base::CreateLogContext clc("update_derivatives");
  DerivativeAccumulator accum;
  internal::SFSetIt<IMP::internal::Stage>
    reset(&cur_stage_, internal::AFTER_EVALUATING);
  boost::timer timer;
  for (int i=states.size()-1; i>=0; --i) {
    ScoreState *ss= states[i];
    IMP_CHECK_OBJECT(ss);
    if (gather_statistics_) timer.restart();
    {
#if IMP_BUILD < IMP_FAST
      if (first_call_) {
        internal::SFResetBitset rbr(Masks::read_mask_, true);
        internal::SFResetBitset rbw(Masks::write_mask_, true);
        internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
        internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
        internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
        ModelObjects inputs=ss->get_inputs();
        ModelObjects outputs=ss->get_outputs();
        Masks::write_mask_.reset();
        IMP_SF_SET_ONLY_2(Masks::read_mask_, inputs, outputs);
        IMP_SF_SET_ONLY_2(Masks::read_derivatives_mask_,inputs, outputs);
        IMP_SF_SET_ONLY_2(Masks::write_derivatives_mask_,inputs, outputs);
        ss->after_evaluate(calc_derivs?&accum:nullptr);
      } else {
        ss->after_evaluate(calc_derivs?&accum:nullptr);
      }
#else
      ss->after_evaluate(calc_derivs?&accum:nullptr);
#endif
    }
    if (gather_statistics_) {
      add_to_update_after_time(ss, timer.elapsed());
    }
    //IMP_LOG(VERBOSE, "." << std::flush);
  }
}

ScoringFunction* Model::create_model_scoring_function() {
  return IMP::create_scoring_function(dynamic_cast<RestraintSet*>(this),
                                      1.0, NO_MAX,
                                      "ModelScoringFunction%1%");
}


double Model::evaluate(bool tf, bool warn) {
  // disable warning for now
  static bool warned=true;
  if (!warned && warn) {
    IMP_WARN("Model::evaluate() is probably not really what you want. "\
             "Consider using IMP::Model::update() if you just want update"\
             " dependencies. "\
             "Or Model::create_scoring_function() and calling evaluate on"\
             " that if you are" \
             " repeatedly evaluating the score.");
    IMP_WARN("Pass false as a second argument to IMP::Model::evaluate() "\
             "if you want to "\
             "disable this warning.");
    warned=true;
  }
  update();
  return RestraintSet::evaluate(tf);
}
IMP_END_NAMESPACE
