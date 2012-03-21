/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/RestraintsScoringFunction.h"
#include "IMP/internal/scoring_functions.h"
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>


#define IMP_SF_CALL_EVALUATE(ninmax, good, usemax)                      \
  std::pair<double, bool> ret;                                          \
  double inmax=ninmax;                                                  \
  if (good) inmax=std::min(inmax, max_);                                \
  if (derivatives) {                                                    \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate<true, good, usemax, true>(ss, rs_, rss_,       \
                                                 inmax,                 \
                                                 get_model());          \
    } else {                                                            \
      ret= exec_evaluate<true, good, usemax, false>(ss, rs_, rss_,      \
                                                  inmax,                \
                                                  get_model());         \
    }                                                                   \
  } else {                                                              \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate<false, good, usemax, true>(ss, rs_, rss_,      \
                                                  inmax,                \
                                                  get_model());         \
    } else {                                                            \
      ret= exec_evaluate<false, good, usemax, false>(ss, rs_, rss_,     \
                                                   inmax,               \
                                                   get_model());        \
    }                                                                   \
  }                                                                     \
  if (!good && ret.first > inmax) {                                     \
    ret.second=false;                                                   \
  }                                                                     \
  ret.first*=weight_;                                                   \
  return ret

IMP_BEGIN_INTERNAL_NAMESPACE

namespace {

template <bool DERIV, bool GOOD, bool MAX, bool STATS, class RS, class RSS>
  inline std::pair<double, bool> exec_evaluate(const RS &restraints,
                                               const RSS &restraint_sets,
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
  for (unsigned int i=0; i< restraints.size(); ++i) {
    std::pair<double, bool> value
        =internal::do_evaluate_restraint<DERIV, GOOD, MAX, STATS>
        (restraints[i].get(),
         remaining,
         initial_weight,
         timer,
         m);
    remaining-=value.first;
    ret+=value.first;
    good= good && value.second;
  }
  for (unsigned int i=0; i< restraint_sets.size(); ++i) {
    std::pair<RestraintsTemp, RestraintSetsTemp> rsp
      = restraint_sets[i]->get_non_sets_and_sets();
    double current_max=std::min(remaining,
                                restraint_sets[i]->get_maximum_score());
    double current_weight= initial_weight
      * restraint_sets[i]->get_weight();
    std::pair<double, bool> value
        = exec_evaluate<DERIV, GOOD, MAX, STATS>(rsp.first, rsp.second,
                                                 current_max,
                                                 current_weight, m);
    remaining-=value.first;
    ret+=value.first;
    good=good&&value.second;
  }
  return std::make_pair(ret, good);
}




  template < bool DERIV, bool GOOD, bool MAX, bool STATS>
  inline std::pair<double, bool> exec_evaluate(const ScoreStatesTemp &states,
                                             const Restraints &restraints,
                                            const RestraintSets &restraint_sets,
                                             double omax,
                                             Model *m) {
    IMP_FUNCTION_LOG;
    IMP_CHECK_OBJECT(m);
    // make sure stage is restored on an exception
    internal::SFSetIt<IMP::internal::Stage, internal::NOT_EVALUATING>
      reset(&m->cur_stage_);
    IMP_CHECK_OBJECT(m);
    IMP_LOG(VERBOSE, "On restraints " << restraints
            << " restraint sets " << restraint_sets
            << " and score states " << states
            << std::endl);
    m->before_evaluate(get_as<ScoreStatesTemp>(states));

    m->cur_stage_= internal::EVALUATING;
    if (DERIV) {
      m->zero_derivatives();
    }
    std::pair<double, bool> ret
        = exec_evaluate< DERIV, GOOD, MAX, STATS>(restraints,
                                              restraint_sets,
                                              omax, 1.0,
                                              m);

    m->after_evaluate(get_as<ScoreStatesTemp>(states), DERIV);

    // validate derivatives
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
      if (DERIV) {
        m->validate_computed_derivatives();
      }
    }
    IMP_LOG(TERSE, "Final score: "
            << ret << std::endl);
    ++m->eval_count_;
    m->first_call_=false;
    return ret;
  }

}


RestraintsScoringFunction::RestraintsScoringFunction(const RestraintsTemp &r,
                                                     double weight,
                                                     double max,
                                                     std::string name):
    ScoringFunction(r[0]->get_model(), name),
    weight_(weight), max_(max){
  for (unsigned int i=0; i< r.size(); ++i) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r[i].get());
    if (rs) {
      rss_.push_back(rs);
    } else {
      rs_.push_back(r[i]);
    }
  }
}


std::pair<double, bool> RestraintsScoringFunction::do_evaluate(bool derivatives,
                                             const ScoreStatesTemp &ss) {
  IMP_SF_CALL_EVALUATE(NO_MAX,
                    false, false);
}
std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_good(bool derivatives,
                                        const ScoreStatesTemp &ss) {
  IMP_SF_CALL_EVALUATE(max_,
                    true, false);
}

std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_below(bool derivatives,
                                         double max,
                                         const ScoreStatesTemp &ss) {
  IMP_SF_CALL_EVALUATE(max,
                    false, true);
}


RestraintsTemp RestraintsScoringFunction::get_restraints() const {
  RestraintsTemp ret= get_as<RestraintsTemp>(rs_)
      + get_as<RestraintsTemp>(rss_);
  return ret;
}


void RestraintsScoringFunction::do_show(std::ostream &out) const {
  IMP_UNUSED(out);
}


IMP_END_INTERNAL_NAMESPACE
