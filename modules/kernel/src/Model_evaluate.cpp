/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Model.h"
#include "IMP/kernel/Particle.h"
#include <IMP/base/log.h>
#include "IMP/kernel/Restraint.h"
#include "IMP/kernel/DerivativeAccumulator.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/generic.h"
#include "IMP/kernel/internal/input_output_exception.h"
#include "IMP/kernel/ScoringFunction.h"
#include "IMP/kernel/internal/evaluate_utility.h"
#include <IMP/base/CreateLogContext.h>
#include <IMP/base/thread_macros.h>
#include <boost/timer.hpp>
#include "IMP/base//set.h"
#include <IMP/base/internal/static.h>
#include <numeric>



IMPKERNEL_BEGIN_NAMESPACE

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


#if IMP_HAS_CHECKS >= IMP_INTERNAL

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2)                         \
  {                                                                     \
    IMP_SF_SET_ONLY(mask, inputs1+inputs2);                             \
  }
#else

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2)
#endif

void Model::before_evaluate(const ScoreStatesTemp &states) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_has_dependencies(),
                  "Model must have dependencies before calling "
                  << "before_evaluate()");
  check_order(states);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  base::internal::check_live_objects();
#endif
  IMP_USAGE_CHECK(cur_stage_== internal::NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating");
  base::CreateLogContext clc("update_score_states");
    internal::SFSetIt<IMP::kernel::internal::Stage>
      reset(&cur_stage_, internal::BEFORE_EVALUATING);
    unsigned int cur_begin=0;
    while (cur_begin < states.size()) {
      unsigned int cur_end=cur_begin+1;
      while (cur_end < states.size()
             && states[cur_begin]->order_ == states[cur_end]->order_) {
        ++cur_end;
      }
      for (unsigned int i=cur_begin; i< cur_end; ++i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
        IMP_LOG_TERSE( "Updating \""
                << ss->get_name() << "\"" << std::endl);
      if ( first_call_) {
          try {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
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
            base::SetNumberOfThreads nt(1);
#endif
            ss->before_evaluate();
          } catch (const internal::InputOutputException &d) {
            IMP_FAILURE(d.get_message(ss));
          }
        } else {
        IMP_TASK((ss),  ss->before_evaluate(),
                 "before evaluate");
        }
      }
IMP_OMP_PRAGMA(taskwait)
IMP_OMP_PRAGMA(flush)
      cur_begin=cur_end;
      //IMP_LOG_VERBOSE( "." << std::flush);
    }
  }

void Model::after_evaluate(const ScoreStatesTemp &istates,
                           const bool calc_derivs) {
  IMP_OBJECT_LOG;
  check_order(istates);
  base::CreateLogContext clc("update_derivatives");
  DerivativeAccumulator accum;
  internal::SFSetIt<IMP::kernel::internal::Stage>
    reset(&cur_stage_, internal::AFTER_EVALUATING);
  unsigned int cur_begin=0;
  ScoreStatesTemp states=istates;
  std::reverse(states.begin(), states.end());
  while (cur_begin < states.size()) {
    unsigned int cur_end=cur_begin+1;
    while (cur_end < states.size()
           && states[cur_begin]->order_ == states[cur_end]->order_) {
      ++cur_end;
    }
    for (unsigned int i=cur_begin; i< cur_end; ++i) {
    ScoreState *ss= states[i];
    IMP_CHECK_OBJECT(ss);
      if ( first_call_) {
        try {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
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
          base::SetNumberOfThreads nt(1);
#endif
          ss->after_evaluate(calc_derivs?&accum:nullptr);
        } catch (const internal::InputOutputException &d) {
          IMP_FAILURE(d.get_message(ss));
        }
      } else {
        IMP_TASK((ss, accum),
                 ss->after_evaluate(calc_derivs? &accum:nullptr),
                 "after evaluate");
      }
    }
IMP_OMP_PRAGMA(taskwait)
IMP_OMP_PRAGMA(flush)
    cur_begin=cur_end;
    }
  }

ScoringFunction* Model::create_model_scoring_function() {
  return IMP::kernel::create_scoring_function(dynamic_cast<RestraintSet*>(this),
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
             "Or Model::create_model_scoring_function() and calling"\
             " evaluate on that if you are repeatedly evaluating" \
             " the score.");
    IMP_WARN("Pass false as a second argument to IMP::Model::evaluate() "\
             "if you want to "\
             "disable this warning.");
    warned=true;
  }
  update();
  return Restraint::evaluate(tf);
}
IMPKERNEL_END_NAMESPACE
