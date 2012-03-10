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
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/generic.h"
#include "IMP/ScoringFunction.h"
#include "IMP/internal/evaluate_utility.h"
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <IMP/base/internal/static.h>
#include <numeric>




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
          internal::SFResetBitset rbr(Masks::read_mask_, true);
          internal::SFResetBitset rbw(Masks::write_mask_, true);
          internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
          internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ParticlesTemp input=ss->get_input_particles();
          ParticlesTemp output=ss->get_output_particles();
          ContainersTemp cinput=ss->get_input_containers();
          ContainersTemp coutput=ss->get_output_containers();
          Masks::read_derivatives_mask_.reset();
          Masks::write_derivatives_mask_.reset();
          IMP_SF_SET_ONLY_2(Masks::read_mask_, input, cinput, output, coutput);
          IMP_SF_SET_ONLY(Masks::write_mask_, output, coutput);
          IMP_SF_SET_ONLY(Masks::add_remove_mask_, output, coutput);
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
          internal::SFResetBitset rbr(Masks::read_mask_, true);
          internal::SFResetBitset rbw(Masks::write_mask_, true);
          internal::SFResetBitset rbar(Masks::add_remove_mask_, true);
          internal::SFResetBitset rbrd(Masks::read_derivatives_mask_, true);
          internal::SFResetBitset rbwd(Masks::write_derivatives_mask_, true);
          ParticlesTemp input=ss->get_input_particles();
          ParticlesTemp output=ss->get_output_particles();
          ContainersTemp cinput=ss->get_input_containers();
          ContainersTemp coutput=ss->get_output_containers();
          Masks::write_mask_.reset();
          IMP_SF_SET_ONLY_2(Masks::read_mask_, input, cinput, output, coutput);
          IMP_SF_SET_ONLY_2(Masks::read_derivatives_mask_,input, cinput, output,
                     coutput);
          IMP_SF_SET_ONLY_2(Masks::write_derivatives_mask_,input, cinput,
                            output,
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
  Pointer<ScoringFunction> sf= create_model_scoring_function();
  return sf->evaluate(calc_derivs);
}

ScoringFunction* Model::create_model_scoring_function() {
  return IMP::create_scoring_function(dynamic_cast<RestraintSet*>(this),
                                      1.0, NO_MAX,
                                      "ModelScoringFunction%1%");
}

IMP_END_NAMESPACE
