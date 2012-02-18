/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/log.h>
#include "IMP/Optimizer.h"
#include "IMP/OptimizerState.h"
#include "IMP/RestraintSet.h"
#include "IMP/dependency_graph.h"
#include "IMP/internal/graph_utility.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <limits>
#include <algorithm>

IMP_BEGIN_NAMESPACE

Optimizer::Optimizer(): Object("Optimizer%1%")
{
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
  stop_on_good_score_=false;
  has_restraints_=false;
  last_score_= std::numeric_limits<double>::max();
}

Optimizer::Optimizer(Model *m, std::string name): Object(name)
{
  if (m) set_model(m);
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
  stop_on_good_score_=false;
  has_restraints_=false;
  last_score_= std::numeric_limits<double>::max();
}

Optimizer::~Optimizer()
{
}

void Optimizer::update_states() const
{
  IMP_LOG(VERBOSE,
          "Updating OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(VERBOSE, "done." << std::endl);
}

double Optimizer::optimize(unsigned int max_steps) {
  IMP_FUNCTION_LOG;
  if (!model_) {
    IMP_THROW("Must give the optimizer a model to optimize",
              ValueException);
  }
  set_was_used(true);

  double ret= do_optimize(max_steps);
  return ret;
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates);

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o)
{
  os->set_optimizer(o);
}


void Optimizer::set_restraints(const RestraintsTemp &rs) {
  cache_= EvaluationCache(rs);
  has_restraints_=true;
}

double Optimizer::evaluate(bool compute_derivatives) const {
  IMP_FUNCTION_LOG;
  if (!has_restraints_) {
    last_score_= get_model()->evaluate(compute_derivatives);
  } else {
    IMP::Floats ret
        = get_model()->evaluate(cache_,
                                compute_derivatives);
    last_score_= std::accumulate(ret.begin(), ret.end(), 0.0);
  }
  return last_score_;
}

double Optimizer::evaluate_if_below(bool compute_derivatives,
                                   double max) const {
  IMP_FUNCTION_LOG;
  Floats ret=get_model()->evaluate_if_below(has_restraints_?
                                            get_model()->get_evaluation_cache():
                                            cache_,
                                            compute_derivatives, max);
  last_score_= std::accumulate(ret.begin(), ret.end(), 0.0);
  return last_score_;
}


RestraintsTemp Optimizer::get_restraints() const {
  if (!has_restraints_) {
    return RestraintsTemp(1, model_->get_root_restraint_set());
  } else {
    return get_as<RestraintsTemp>(cache_.get_restraints());
  }
}

IMP_END_NAMESPACE
