/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include <IMP/base/log.h>
#include "IMP/Restraint.h"
#include "IMP/internal/restraint_evaluation.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <IMP/base/internal/static.h>
#include <numeric>



#define IMP_CALL_EVALUATE(ss, rs, weights, max, derivs, good, usemax) \
  if (derivs) {                                                \
    if (gather_statistics_) {                                           \
      ret= internal::evaluate<true, good, usemax, true>(ss,             \
                                                        rs,             \
                                                        weights,        \
                                                        max,            \
                                                        this);          \
  } else {                                                              \
      ret= internal::evaluate<true, good, usemax, false>(ss,            \
                                                         rs,            \
                                                         weights,       \
                                                         max,           \
                                                         this);         \
    }                                                                   \
  } else {                                                              \
    if (gather_statistics_) {                                           \
      ret= internal::evaluate<false, good, usemax, true>(ss,            \
                                                        rs,             \
                                                        weights,        \
                                                        max,            \
                                                        this);          \
    } else {                                                            \
      ret= internal::evaluate<false, good, usemax, false>(ss,            \
                                                         rs,            \
                                                         weights,       \
                                                         max,           \
                                                         this);         \
    }                                                                   \
  }


IMP_BEGIN_NAMESPACE


void Model::before_evaluate(const ScoreStatesTemp &states) {
#if IMP_BUILD < IMP_FAST
  base::internal::check_live_objects();
#endif
  IMP_USAGE_CHECK(cur_stage_== internal::NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating");
  CreateLogContext clc("update_score_states");
  {
    cur_stage_= internal::BEFORE_EVALUATING;
    boost::timer timer;
    for (unsigned int i=0; i< states.size(); ++i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      {
#if IMP_BUILD < IMP_FAST
        if (first_call_) {
          internal::ResetBitset rbr(Masks::read_mask_, true);
          internal::ResetBitset rbw(Masks::write_mask_, true);
          internal::ResetBitset rbar(Masks::add_remove_mask_, true);
          internal::ResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::ResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ParticlesTemp input=ss->get_input_particles();
          ParticlesTemp output=ss->get_output_particles();
          ContainersTemp cinput=ss->get_input_containers();
          ContainersTemp coutput=ss->get_output_containers();
          Masks::read_derivatives_mask_.reset();
          Masks::write_derivatives_mask_.reset();
          IMP_SET_ONLY_2(Masks::read_mask_, input, cinput, output, coutput);
          IMP_SET_ONLY(Masks::write_mask_, output, coutput);
          IMP_SET_ONLY(Masks::add_remove_mask_, output, coutput);
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
  CreateLogContext clc("update_derivatives");
  {
    DerivativeAccumulator accum;
    cur_stage_= internal::AFTER_EVALUATING;
    boost::timer timer;
    for (int i=states.size()-1; i>=0; --i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      if (gather_statistics_) timer.restart();
      {
#if IMP_BUILD < IMP_FAST
        if (first_call_) {
          internal::ResetBitset rbr(Masks::read_mask_, true);
          internal::ResetBitset rbw(Masks::write_mask_, true);
          internal::ResetBitset rbar(Masks::add_remove_mask_, true);
          internal::ResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::ResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ParticlesTemp input=ss->get_input_particles();
          ParticlesTemp output=ss->get_output_particles();
          ContainersTemp cinput=ss->get_input_containers();
          ContainersTemp coutput=ss->get_output_containers();
          Masks::write_mask_.reset();
          IMP_SET_ONLY_2(Masks::read_mask_, input, cinput, output, coutput);
          IMP_SET_ONLY_2(Masks::read_derivatives_mask_,input, cinput, output,
                     coutput);
          IMP_SET_ONLY_2(Masks::write_derivatives_mask_,input, cinput, output,
                     coutput);
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
}



double Model::evaluate(bool calc_derivs) {
  Floats all=evaluate(get_evaluation_cache(), calc_derivs);
  return std::accumulate(all.begin(), all.end(), 0.0);
}


Floats Model::evaluate( const EvaluationCache &cache,
                        bool calc_derivs)
{
  Floats ret;
  IMP_CALL_EVALUATE(cache.ss_,
                    cache.rs_,
                    internal::ExternalWeights(cache.weights_),
                    std::numeric_limits<double>::max(),
                    calc_derivs, false, false);
  return ret;
}

Floats Model::evaluate_if_below( const EvaluationCache &cache,
                                 bool calc_derivs,
                                 double max)
{
  Floats ret;
  IMP_CALL_EVALUATE(cache.ss_,
                    cache.rs_,
                    internal::ExternalWeights(cache.weights_),
                    max,
                    calc_derivs, false, true);
  return ret;
}

Floats Model::evaluate_if_good( const EvaluationCache &cache,
                                bool calc_derivs)
{
  Floats ret;
  IMP_CALL_EVALUATE(cache.ss_,
                    cache.rs_,
                    internal::ExternalWeights(cache.weights_),
                    get_maximum_score(),
                    calc_derivs, true, false);
  return ret;
}

IMP_END_NAMESPACE
