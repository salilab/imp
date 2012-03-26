/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H
#define IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H

#include "../kernel_config.h"
#include "../ScoringFunction.h"
#include "../Model.h"
#include "../scoring_function_macros.h"
#include "RestraintsScoringFunction.h"
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>

#include "evaluate_utility.h"
#include "utility.h"

#if IMP_BUILD < IMP_FAST

#define IMP_WRAP_SF_EVALUATE_CALL(restraint, expr, m)                   \
  if (m->first_call_) {                                                 \
    internal::SFResetBitset rbr(m->Masks::read_mask_, true);            \
    internal::SFResetBitset rbw(m->Masks::write_mask_, true);           \
    internal::SFResetBitset rbar(m->Masks::add_remove_mask_, true);     \
    internal::SFResetBitset rbrd(m->Masks::read_derivatives_mask_, true); \
    internal::SFResetBitset rbwd(m->Masks::write_derivatives_mask_, true); \
    m->Masks::write_mask_.reset();                                      \
    m->Masks::add_remove_mask_.reset();                                 \
    m->Masks::read_derivatives_mask_.reset();                           \
    IMP_SF_SET_ONLY(m->Masks::read_mask_, restraint->get_input_particles(), \
                 restraint->get_input_containers()                      \
                 );                                                     \
    IMP_SF_SET_ONLY(m->Masks::write_derivatives_mask_,                  \
                 restraint->get_input_particles(),                      \
                 restraint->get_input_containers()                      \
                 );                                                     \
    IMP_SF_SET_ONLY(m->Masks::read_derivatives_mask_,                   \
                 restraint->get_input_particles(),                      \
                 restraint->get_input_containers()                      \
                 );                                                     \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  } else {                                                              \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  }                                                                     \

#else
#define IMP_WRAP_SF_EVALUATE_CALL(restraint, expr, m) expr
#endif




#define IMP_CALL_EVALUATE_ONE(max, good, usemax)                        \
  std::pair<double, bool> ret;                                          \
  if (derivatives) {                                                    \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate_one<true, good, usemax, true>(ss, r_.get(),    \
                                                     max,               \
                                                     get_model());      \
    } else {                                                            \
      ret= exec_evaluate_one<true, good, usemax, false>(ss, r_.get(),   \
                                                      max,              \
                                                      get_model());     \
    }                                                                   \
  } else {                                                              \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate_one<false, good, usemax, true>(ss, r_.get(),   \
                                                      max,              \
                                                      get_model());     \
    } else {                                                            \
      ret= exec_evaluate_one<false, good, usemax, false>(ss, r_.get(),  \
                                                       max,             \
                                                       get_model());    \
    }                                                                   \
  }                                                                     \
  return ret

#define IMP_CALL_EVALUATE_ONE_WRAPPED(ninmax, good, usemax)              \
  std::pair<double, bool> ret;                                          \
  double inmax=ninmax;                                                  \
  if (good) inmax=std::min(inmax, max_);                                \
  if (derivatives) {                                                    \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate_one<true, good, usemax, true>(ss, r_.get(),    \
                                                     inmax,             \
                                                     get_model());      \
    } else {                                                            \
      ret= exec_evaluate_one<true, good, usemax, false>(ss, r_.get(),   \
                                                      inmax,            \
                                                      get_model());     \
    }                                                                   \
  } else {                                                              \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate_one<false, good, usemax, true>(ss, r_.get(),   \
                                                      inmax,            \
                                                      get_model());     \
    } else {                                                            \
      ret= exec_evaluate_one<false, good, usemax, false>(ss, r_.get(),  \
                                                       inmax,           \
                                                       get_model());    \
    }                                                                   \
  }                                                                     \
  if (!good && ret.first > max_) {                                      \
    ret.second=false;                                                   \
  }                                                                     \
  ret.first*=weight_;                                                   \
  return ret

IMP_BEGIN_INTERNAL_NAMESPACE

template <bool DERIV, bool GOOD, bool MAX, bool STATS, class RS>
inline std::pair<double, bool> do_evaluate_restraint(RS *r,
                                                     double max,
                                                     double initial_weight,
                                        boost::scoped_ptr<boost::timer> &timer,
                                                     Model *m) {
  double value=0;
  double weight=initial_weight*r->get_weight();
  DerivativeAccumulator accum(weight);
  double current_max=max/weight;
  if (GOOD) {
    current_max=std::max(current_max,
                         r->get_maximum_score());
  }
  if (STATS) timer->restart();
  if (GOOD) {
    IMP_WRAP_SF_EVALUATE_CALL(r,
                           value=
                           r->unprotected_evaluate_if_good(DERIV?
                                                           &accum:nullptr,
                                                           current_max),
                           m);
  } else if (MAX) {
    IMP_WRAP_SF_EVALUATE_CALL(r,
                           value=
                           r->unprotected_evaluate_if_good(DERIV?
                                                           &accum:nullptr,
                                                           current_max),
                           m);
  } else {
    IMP_WRAP_SF_EVALUATE_CALL(r,
                           value=
                           r->unprotected_evaluate(DERIV?
                                                   &accum:nullptr),
                           m);
  }
  r->set_was_used(true);
  r->last_score_=value;
  if (m->gather_statistics_) {
    m->add_to_restraint_evaluate(r, timer->elapsed(), value);
  }
  bool good= (value > current_max);
  if (value> current_max &&(MAX || GOOD)) {
    value= BAD_SCORE;
  } else {
    value*= weight;
  }
  IMP_LOG(TERSE, r->get_name()<<  " score is "
          << value << " (" << weight << ")" << std::endl);
  return std::make_pair(value, good);
}






template <bool DERIV, bool GOOD, bool MAX, bool STATS, class RS>
inline std::pair<double, bool> exec_evaluate_one( RS *restraint,
                                                double max,
                                                double initial_weight,
                                                Model *m) {
  IMP_FUNCTION_LOG;
  BOOST_STATIC_ASSERT((!GOOD || !MAX));
  double ret=0;
  double remaining=max;
  boost::scoped_ptr<boost::timer> timer;
  bool good=true;
  if (STATS) timer.reset(new boost::timer());
  std::pair<double, bool> value
      =do_evaluate_restraint<DERIV, GOOD, MAX, STATS>(restraint,
                                                      remaining,
                                                      initial_weight,
                                                      timer,
                                                      m);
  remaining-=value.first;
  ret+=value.first;
  good= good && value.second;
  return std::make_pair(ret, good);
}




template < bool DERIV, bool GOOD, bool MAX, bool STATS, class RS>
inline std::pair<double, bool> exec_evaluate_one(const ScoreStatesTemp &states,
                                               RS *restraint,
                                               double omax,
                                               Model *m) {
  IMP_FUNCTION_LOG;
  IMP_CHECK_OBJECT(m);
  IMP_LOG(VERBOSE, "On restraints " << Showable(restraint)
          << " and score states " << states
          << std::endl);
  m->before_evaluate(states);
  internal::SFSetIt<IMP::internal::Stage>
      reset(&m->cur_stage_, internal::EVALUATING);
  // make sure stage is restored on an exception
  if (DERIV) {
    m->zero_derivatives();
  }
  std::pair<double, bool> ret
    = exec_evaluate_one< DERIV, GOOD, MAX, STATS>(restraint,
                                                  omax, 1.0,
                                                  m);
  m->after_evaluate(states, DERIV);

  // validate derivatives
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    if (DERIV) {
      m->validate_computed_derivatives();
    }
  }
  IMP_LOG(TERSE, "Final score: " << Showable(ret) << std::endl);
  ++m->eval_count_;
  m->first_call_=false;
  return ret;
}




inline Model *get_model(const RestraintsTemp &rs) {
  IMP_USAGE_CHECK(!rs.empty(),
                  "Can't pass empty list of restraints as we"
                  << " need to find the model.");
  Model *m= rs[0]->get_model();
  IMP_USAGE_CHECK(m, "Model was not set on restraint "
                  << Showable(rs[0]) << " please set it before"
                  << " creating ScoringFunction.");
  return m;
}


inline Model *get_model( Restraint* rs) {
  IMP_USAGE_CHECK(rs,
                  "Can't null restraint as we"
                  << " need to find the model.");
  Model *m= rs->get_model();
  IMP_USAGE_CHECK(m, "Model was not set on restraint "
                  << Showable(rs) << " please set it before"
                  << " creating ScoringFunction.");
  return m;
}





/** Implement a scoring function on a single restraint.
 */
template <class RestraintType>
class RestraintScoringFunction: public ScoringFunction {
  base::OwnerPointer<RestraintType> r_;
 public:
  RestraintScoringFunction(RestraintType* r):
    ScoringFunction(IMP::internal::get_model(r),
                    r->get_name()+"SF"), r_(r){}
  RestraintScoringFunction(RestraintType* r,
                           std::string name):
    ScoringFunction(IMP::internal::get_model(r), name), r_(r){}
  IMP_SCORING_FUNCTION(RestraintScoringFunction);
};

template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate(bool derivatives,
              const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE(NO_MAX,
                        false, false);
}
template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate_if_good(bool derivatives,
                      const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE(NO_MAX,
                        true, false);
}

template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate_if_below(bool derivatives,
                       double max,
                       const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE(max,
                        false, true);
}
template <class RestraintType>
Restraints RestraintScoringFunction<RestraintType>::create_restraints() const {
  return Restraints(1, r_);
}

template <class RestraintType>
void RestraintScoringFunction<RestraintType>::do_show(std::ostream &out) const {
  IMP_UNUSED(out);
}


template <class RestraintType>
ScoreStatesTemp
RestraintScoringFunction<RestraintType>
::get_required_score_states(const DependencyGraph &) const {
  return get_model()->get_score_states(RestraintsTemp(1,r_));
}




/** Implement a scoring function on a single restraint.
 */
template <class RestraintType>
class WrappedRestraintScoringFunction: public ScoringFunction {
  base::OwnerPointer<RestraintType> r_;
  double weight_;
  double max_;
 public:
  WrappedRestraintScoringFunction(RestraintType* r,
                                  double weight,
                                  double max):
    ScoringFunction(IMP::internal::get_model(r),
                    r->get_name()+"SF"), r_(r),
      weight_(weight), max_(max){}
  WrappedRestraintScoringFunction(RestraintType* r,
                                  double weight,
                                  double max,
                                  std::string name):
    ScoringFunction(IMP::internal::get_model(r),
                      name), r_(r),
      weight_(weight), max_(max){}
  IMP_SCORING_FUNCTION(WrappedRestraintScoringFunction);
};

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate(bool derivatives,
              const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE_WRAPPED(NO_MAX,
                                false, false);
}

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate_if_good(bool derivatives,
                      const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE_WRAPPED(max_,
                                true, false);
}

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate_if_below(bool derivatives,
                       double max,
                       const ScoreStatesTemp &ss) {
  IMP_CALL_EVALUATE_ONE_WRAPPED(max,
                                false, true);
}
template <class RestraintType>
Restraints
WrappedRestraintScoringFunction<RestraintType>::create_restraints() const {
  IMP_NEW(RestraintSet, rs, (get_name()+" weights"));
  rs->add_restraint(r_);
  rs->set_model(get_model());
  rs->set_maximum_score(max_);
  rs->set_weight(weight_);
  return Restraints(1, rs);
}

template <class RestraintType>
ScoreStatesTemp
WrappedRestraintScoringFunction<RestraintType>
::get_required_score_states(const DependencyGraph &) const {
  return get_model()->get_score_states(RestraintsTemp(1,r_));
}

template <class RestraintType>
void
WrappedRestraintScoringFunction<RestraintType>::do_show(std::ostream &out)
    const {
  IMP_UNUSED(out);
}



/** Create a ScoringFunction on a single restraints.*/
template <class RestraintType>
inline ScoringFunction* create_scoring_function(RestraintType* rs,
                                               double weight=1.0,
                                                double max=NO_MAX,
                                                std::string name=
                                                std::string()) {
  if (name.empty()) {
    name= rs->get_name()+"ScoringFunction";
  }
  if (dynamic_cast<RestraintSet*>(rs)) {
    RestraintSet *rrs=dynamic_cast<RestraintSet*>(rs);
    if (rrs->get_number_of_restraints()==0) {
      // ick
      return new RestraintsScoringFunction(RestraintsTemp(1,rs), weight, max,
                                           name);
    }
    return new RestraintsScoringFunction(RestraintsTemp(rrs->restraints_begin(),
                                                        rrs->restraints_end()),
                                         weight*rs->get_weight(),
                                         std::min(max,
                                                  rs->get_maximum_score()),
                                         name);
  } else {
    if (weight==1.0 && max==NO_MAX) {
      return new internal::RestraintScoringFunction<RestraintType>(rs,
                                                                   name);
    } else {
      return new internal::WrappedRestraintScoringFunction<RestraintType>(rs,
                                                                        weight,
                                                                          max,
                                                                          name);
    }
  }
}


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H */
