/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include <IMP/log.h>
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/generic.h"
#include "IMP/internal/input_output_exception.h"
#include "IMP/ScoringFunction.h"
#include "IMP/internal/evaluate_utility.h"
#include <IMP/CreateLogContext.h>
#include <IMP/thread_macros.h>
#include <boost/timer.hpp>
#include <IMP/internal/base_static.h>
#include <numeric>

IMPKERNEL_BEGIN_NAMESPACE

namespace {
void check_order(const ScoreStatesTemp &ss) {
  for (unsigned int i = 1; i < ss.size(); ++i) {
    IMP_USAGE_CHECK(ss[i - 1]->get_update_order() <= ss[i]->get_update_order(),
                    "Score states " << Showable(ss[i - 1]) << " and "
                                    << Showable(ss[i]) << " are out of order.");
  }
}
}

#if IMP_HAS_CHECKS >= IMP_INTERNAL

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2) \
  { IMP_SF_SET_ONLY(mask, inputs1 + inputs2); }
#else

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2)
#endif

void Model::before_evaluate(const ScoreStatesTemp &states) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_has_all_dependencies(),
                  "Model must have dependencies before calling "
                      << "before_evaluate()");
  check_order(states);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  internal::check_live_objects();
#endif
  IMP_USAGE_CHECK(cur_stage_ == internal::NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating");
  CreateLogContext clc("update_score_states");
  internal::SFSetIt<IMP::internal::Stage> reset(
      &cur_stage_, internal::BEFORE_EVALUATING);
  unsigned int cur_begin = 0;
  while (cur_begin < states.size()) {
    unsigned int cur_end = cur_begin + 1;
    while (cur_end < states.size() && states[cur_begin]->get_update_order() ==
                                          states[cur_end]->get_update_order()) {
      ++cur_end;
    }
    for (unsigned int i = cur_begin; i < cur_end; ++i) {
      ScoreState *ss = states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG_TERSE("Updating \"" << ss->get_name() << "\"" << std::endl);
      if (first_call_) {
        try {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
          internal::SFResetBitset rbr(Masks::read_mask_, true);
          internal::SFResetBitset rbw(Masks::write_mask_, true);
          internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
          internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ModelObjects inputs = ss->get_inputs();
          ModelObjects outputs = ss->get_outputs();
          Masks::read_derivatives_mask_.reset();
          Masks::write_derivatives_mask_.reset();
          IMP_SF_SET_ONLY_2(Masks::read_mask_, inputs, outputs);
          IMP_SF_SET_ONLY(Masks::write_mask_, outputs);
          IMP_SF_SET_ONLY(Masks::add_remove_mask_, outputs);
          SetNumberOfThreads nt(1);
#endif
          ss->before_evaluate();
        }
        catch (const internal::InputOutputException &d) {
          IMP_FAILURE(d.get_message(ss));
        }
      } else {
        IMP_TASK((ss), ss->before_evaluate(), "before evaluate");
      }
    }
    IMP_OMP_PRAGMA(taskwait)
    IMP_OMP_PRAGMA(flush)
    cur_begin = cur_end;
    // IMP_LOG_VERBOSE( "." << std::flush);
  }
}

void Model::after_evaluate(const ScoreStatesTemp &istates,
                           const bool calc_derivs) {
  IMP_OBJECT_LOG;
  check_order(istates);
  CreateLogContext clc("update_derivatives");
  DerivativeAccumulator accum;
  internal::SFSetIt<IMP::internal::Stage> reset(
      &cur_stage_, internal::AFTER_EVALUATING);
  unsigned int cur_begin = 0;
  ScoreStatesTemp states = istates;
  std::reverse(states.begin(), states.end());
  while (cur_begin < states.size()) {
    unsigned int cur_end = cur_begin + 1;
    while (cur_end < states.size() && states[cur_begin]->get_update_order() ==
                                          states[cur_end]->get_update_order()) {
      IMP_CHECK_OBJECT(states[cur_end]);
      ++cur_end;
    }
    for (unsigned int i = cur_begin; i < cur_end; ++i) {
      ScoreState *ss = states[i];
      IMP_CHECK_OBJECT(ss);
      if (first_call_) {
        try {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
          internal::SFResetBitset rbr(Masks::read_mask_, true);
          internal::SFResetBitset rbw(Masks::write_mask_, true);
          internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
          internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ModelObjects inputs = ss->get_inputs();
          ModelObjects outputs = ss->get_outputs();
          Masks::write_mask_.reset();
          IMP_SF_SET_ONLY_2(Masks::read_mask_, inputs, outputs);
          IMP_SF_SET_ONLY_2(Masks::read_derivatives_mask_, inputs, outputs);
          IMP_SF_SET_ONLY_2(Masks::write_derivatives_mask_, inputs, outputs);
          SetNumberOfThreads nt(1);
#endif
          ss->after_evaluate(calc_derivs ? &accum : nullptr);
        }
        catch (const internal::InputOutputException &d) {
          IMP_FAILURE(d.get_message(ss));
        }
      } else {
/* gcc 9 requires that we make calc_derivs a shared variable so each task
   can see it. gcc 8 automatically shares const variables and reports an error
   if we try to explicitly share one. The same is true for clang 10 vs 9. */
#if defined(__clang__)
#if __clang_major__ >= 10
        IMP_TASK_SHARED((ss, accum), (calc_derivs),
                 ss->after_evaluate(calc_derivs ? &accum : nullptr),
                 "after evaluate");
#else
        IMP_TASK((ss, accum),
                 ss->after_evaluate(calc_derivs ? &accum : nullptr),
                 "after evaluate");
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 9
        IMP_TASK_SHARED((ss, accum), (calc_derivs),
                 ss->after_evaluate(calc_derivs ? &accum : nullptr),
                 "after evaluate");
#else
        IMP_TASK((ss, accum),
                 ss->after_evaluate(calc_derivs ? &accum : nullptr),
                 "after evaluate");
#endif
#endif
      }
    }
    IMP_OMP_PRAGMA(taskwait)
    IMP_OMP_PRAGMA(flush)
    cur_begin = cur_end;
  }
}

IMPKERNEL_END_NAMESPACE
