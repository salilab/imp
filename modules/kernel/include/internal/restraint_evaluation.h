/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H
#define IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H

#include "../kernel_config.h"
#include "../ScoringFunction.h"
#include "../Model.h"
#include "../container_base.h"
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>

#include "evaluate_utility.h"
#include "utility.h"

#if IMP_BUILD < IMP_FAST

#define IMP_WRAP_SF_EVALUATE_CALL(restraint, expr, m)                   \
  if (m->first_call_) {                                                 \
    IMP_LOG_CONTEXT(restraint->get_name() + " evaluation");             \
    IMP_LOG(TERSE, "Checking dependencies." << std::endl);              \
    internal::SFResetBitset rbr(m->Masks::read_mask_, true);            \
    internal::SFResetBitset rbw(m->Masks::write_mask_, true);           \
    internal::SFResetBitset rbar(m->Masks::add_remove_mask_, true);     \
    internal::SFResetBitset rbrd(m->Masks::read_derivatives_mask_, true); \
    internal::SFResetBitset rbwd(m->Masks::write_derivatives_mask_, true); \
    m->Masks::write_mask_.reset();                                      \
    m->Masks::add_remove_mask_.reset();                                 \
    m->Masks::read_derivatives_mask_.reset();                           \
    IMP_SF_SET_ONLY(m->Masks::read_mask_, restraint->get_inputs()       \
                 );                                                     \
    IMP_SF_SET_ONLY(m->Masks::write_derivatives_mask_,                  \
                    restraint->get_inputs());                           \
    IMP_SF_SET_ONLY(m->Masks::read_derivatives_mask_,                   \
                    restraint->get_inputs());                           \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  } else {                                                              \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  }                                                                     \

#else
#define IMP_WRAP_SF_EVALUATE_CALL(restraint, expr, m) expr
#endif



#define IMP_SF_CALL_UNPROTECTED_EVALUATE(good, usemax, derivatives, call) \
  std::pair<double, bool> ret;                                          \
  {                                                                     \
    internal::SFSetIt<IMP::internal::Stage>                               \
        reset(&get_model()->cur_stage_, internal::EVALUATING);          \
    if (derivatives) {                                                  \
      if (get_model()->get_gather_statistics()) {                       \
        ret= IMP::internal::EvaluationEnvironment<true, good, usemax, true> \
            ::call;                                                     \
      } else {                                                          \
        ret= IMP::internal::EvaluationEnvironment<true, good, usemax, false> \
            ::call;                                                     \
      }                                                                 \
    } else {                                                            \
      if (get_model()->get_gather_statistics()) {                       \
        ret= IMP::internal::EvaluationEnvironment<false, good, usemax, true> \
            ::call;                                                     \
      } else {                                                          \
        ret= IMP::internal::EvaluationEnvironment<false, good, usemax, false> \
            ::call;                                                     \
      }                                                                 \
    }                                                                   \
  }

#define IMP_SF_CALL_PROTECTED_EVALUATE(good, usemax, derivatives, \
                                       call, weight,                    \
                                       states)                          \
  IMP::internal::before_protected_evaluate(get_model(), states, derivatives); \
  IMP_SF_CALL_UNPROTECTED_EVALUATE(good, usemax, derivatives, call);    \
  ret.first*= weight;                                                   \
  IMP::internal::after_protected_evaluate(get_model(), states, derivatives)


IMP_BEGIN_INTERNAL_NAMESPACE

template <bool DERIV, bool GOOD, bool MAX, bool STATS>
struct EvaluationEnvironment {
  template <class RS>
  inline static std::pair<double, bool> unprotected_evaluate(RS *r,
                                                             double max,
                                                    double derivative_weight,
                                                             Model *m) {
    boost::scoped_ptr<boost::timer> timer;
    if (STATS) timer.reset(new boost::timer());

    double value=0;
    DerivativeAccumulator accum(derivative_weight);
    if (STATS) timer->restart();
    if (GOOD) {
      IMP_WRAP_SF_EVALUATE_CALL(r,
                                value=
                                r->unprotected_evaluate_if_good(DERIV?
                                                                &accum:nullptr,
                                                                max),
                                m);
    } else if (MAX) {
      IMP_WRAP_SF_EVALUATE_CALL(r,
                                value=
                                r->unprotected_evaluate_if_good(DERIV?
                                                                &accum:nullptr,
                                                                max),
                                m);
    } else {
      IMP_WRAP_SF_EVALUATE_CALL(r,
                                value=
                                r->unprotected_evaluate(DERIV?
                                                        &accum:nullptr),
                                m);
    }
    r->set_was_used(true);
    r->set_last_score(value);
    if (STATS) {
      m->add_to_restraint_evaluate(r, timer->elapsed(), value);
    }
    bool good= (value < max);
    if (!good &&(MAX || GOOD)) {
      value= BAD_SCORE;
    }
    IMP_LOG(TERSE, Showable(r) <<  " score is "
            << value << std::endl);
    return std::make_pair(value, good);
  }

  template <class RS, class RSS>
  inline static std::pair<double, bool>
  unprotected_evaluate(const RS &restraints,
                       const RSS &restraint_sets,
                       double max,
                       double derivative_weight,
                       Model *m) {
    IMP_FUNCTION_LOG;
    BOOST_STATIC_ASSERT((!GOOD || !MAX));
    double ret=0;
    double remaining=max;
    bool good=true;
    for (unsigned int i=0; i< restraints.size(); ++i) {
      double weight= restraints[i]->get_weight();
      double current_max=std::min(remaining/weight,
                                  restraints[i]->get_maximum_score());
      std::pair<double, bool> value
          =unprotected_evaluate(restraints[i].get(),current_max,
                                derivative_weight*weight, m);
      double weighted_value= value.first*weight;
      remaining-=weighted_value;
      ret+=weighted_value;
      good= good && value.second;
      if (!good && (MAX || GOOD)) {
        return std::make_pair(NO_MAX, false);
      }
    }
    for (unsigned int i=0; i< restraint_sets.size(); ++i) {
      double weight= restraint_sets[i]->get_weight();
      double current_max=std::min(remaining/weight,
                                  restraint_sets[i]->get_maximum_score());
      std::pair<RestraintsTemp, RestraintSetsTemp> rsp
          = restraint_sets[i]->get_non_sets_and_sets();
      std::pair<double, bool> value
          = unprotected_evaluate(rsp.first, rsp.second,
                                 current_max,
                                 derivative_weight*weight,
                                 m);
      restraint_sets[i]->set_last_score(value.first);
      double weighted_value= value.first*weight;
      remaining-=weighted_value;
      ret+=weighted_value;
      good= good && value.second;
      if (!good && (MAX || GOOD)) {
        return std::make_pair(NO_MAX, false);
      }
    }
    return std::make_pair(ret, good);
  }

};

  inline static void before_protected_evaluate(Model *m,
                                               const ScoreStatesTemp &states,
                                               bool derivative) {
    IMP_CHECK_OBJECT(m);
    m->before_evaluate(states);
    if (derivative) {
      m->zero_derivatives();
    }
  }
  inline static void after_protected_evaluate(Model *m,
                                              const ScoreStatesTemp &states,
                                              bool derivative) {
    m->after_evaluate(states, derivative);
    // validate derivatives
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
      if (derivative) {
        m->validate_computed_derivatives();
      }
    }
    ++m->eval_count_;
    m->first_call_=false;
  }

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H */
