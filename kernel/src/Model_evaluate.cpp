/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <numeric>


namespace {
  template <class T, int V>
  struct SetIt {
    T *t_;
    SetIt(T *t): t_(t){}
    ~SetIt() {
      *t_= T(V);
    }
  };
}

#if IMP_BUILD < IMP_FAST
#define WRAP_UPDATE_CALL(restraint, expr, exchange)                     \
  expr

#define WRAP_EVALUATE_CALL(restraint, expr)                             \
  expr

#else
#define WRAP_UPDATE_CALL(restraint, expr, exchange) expr
#define WRAP_EVALUATE_CALL(restraint, expr) expr

#endif

IMP_BEGIN_NAMESPACE


void Model::before_evaluate(const ScoreStatesTemp &states) const {
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
      WRAP_UPDATE_CALL(ss, ss->before_evaluate(), false);
      if (gather_statistics_) {
        add_to_update_before_time(ss, timer.elapsed());
      }
      //IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
}

void Model::after_evaluate(const ScoreStatesTemp &states,
                           bool calc_derivs) const {
  CreateLogContext clc("update_derivatives");
  {
    DerivativeAccumulator accum;
    cur_stage_= internal::AFTER_EVALUATING;
    boost::timer timer;
    for (int i=states.size()-1; i>=0; --i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Post updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      WRAP_UPDATE_CALL(ss, ss->after_evaluate(calc_derivs?&accum:NULL), true);
      if (gather_statistics_) {
        add_to_update_after_time(ss, timer.elapsed());
      }
      //IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
}


Floats Model::do_evaluate_restraints(const RestraintsTemp &restraints,
                                     bool calc_derivs,
                                     bool if_good, bool if_max,
                                     double omax) {
  IMP_FUNCTION_LOG;
  IMP_INTERNAL_CHECK(!if_good || !if_max, "Can't be both max and good");
  Floats ret;
  double remaining=omax;
  if (!if_max) {
    remaining=get_maximum_score();
  }
  boost::timer timer;
  const unsigned int rsz=restraints.size();
  for (unsigned int i=0; i< rsz; ++i) {
    double value=0;
    double weight=restraints[i]->model_weight_;
    DerivativeAccumulator accum(weight);
    if (gather_statistics_) timer.restart();
    if (if_good) {
      double max=std::min(remaining,
                          restraints[i]->get_maximum_score());
      WRAP_EVALUATE_CALL(restraints[i],
                         value=
                   restraints[i]->unprotected_evaluate_if_good(calc_derivs?
                                                              &accum:NULL,
                                                                   max));
    } else if (if_max) {
      WRAP_EVALUATE_CALL(restraints[i],
                         value=
                   restraints[i]->unprotected_evaluate_if_good(calc_derivs?
                                                              &accum:NULL,
                                                                   remaining));
    } else {
      WRAP_EVALUATE_CALL(restraints[i],
                         value=
                         restraints[i]->unprotected_evaluate(calc_derivs?
                                                             &accum:NULL));
    }
    double wvalue= weight*value;
    remaining-=wvalue;
    IMP_LOG(TERSE, restraints[i]->get_name()<<  " score is "
              << wvalue << std::endl);
    if (gather_statistics_) {
      add_to_restraint_evaluate(restraints[i], timer.elapsed(), wvalue);
    }
    if (value > restraints[i]->get_maximum_score()) {
      has_good_score_=false;
    }
    ret.push_back(wvalue);
  }
  if (remaining<0) {
    has_good_score_=false;
  }
  return ret;
}






Floats Model::do_evaluate(const RestraintsTemp &restraints,
                          const ScoreStatesTemp &states,
                          bool calc_derivs,
                          bool if_good, bool if_max, double max) {
  // make sure stage is restored on an exception
  SetIt<IMP::internal::Stage, internal::NOT_EVALUATING> reset(&cur_stage_);
  IMP_CHECK_OBJECT(this);
  IMP_LOG(VERBOSE, "On restraints " << Restraints(restraints)
          << " and score states " << ScoreStates(states)
          << std::endl);

  before_evaluate(states);

  cur_stage_= internal::EVALUATING;
  if (calc_derivs) {
    zero_derivatives();
  }
  std::vector<double> ret= do_evaluate_restraints(restraints,
                                                  calc_derivs,
                                                  if_good, if_max,
                                                  max);

  after_evaluate(states, calc_derivs);

  // validate derivatives
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (calc_derivs) {
      validate_computed_derivatives();
    }
  }
  IMP_LOG(TERSE, "Final score: "
          << std::accumulate(ret.begin(), ret.end(), 0.0) << std::endl);
  cur_stage_=internal::NOT_EVALUATING;
  ++eval_count_;
  first_call_=false;
  return ret;
}


IMP_END_NAMESPACE
