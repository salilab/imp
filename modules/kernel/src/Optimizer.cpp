/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base//log.h>
#include "IMP/kernel/Optimizer.h"
#include "IMP/kernel/OptimizerState.h"
#include "IMP/kernel/RestraintSet.h"
#include "IMP/kernel/generic.h"
#include "IMP/kernel/dependency_graph.h"
#include "IMP/kernel/internal/graph_utility.h"
#include "IMP/kernel/internal/RestraintsScoringFunction.h"
#include "IMP/kernel/internal/container_helpers.h"
#include <IMP/base//thread_macros.h>
#include "IMP/kernel/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <limits>
#include <algorithm>

IMPKERNEL_BEGIN_NAMESPACE

Optimizer::Optimizer(): Object("Optimizer%1%")
{
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
  stop_on_good_score_=false;
}

Optimizer::Optimizer(Model *m, std::string name): Object(name)
{
  if (m) set_model(m);
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
  stop_on_good_score_=false;
}

Optimizer::~Optimizer()
{
}

void Optimizer::set_model(Model *m) {
  cache_= m->create_model_scoring_function();
  cache_->set_was_used(true);
  model_=m;
}

void Optimizer::update_states() const
{
  IMP_LOG_VERBOSE(
          "Updating OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG_VERBOSE( "." << std::flush);
  }
  IMP_LOG_VERBOSE( "done." << std::endl);
}

void Optimizer::set_is_optimizing_states(bool tf) const
{
  IMP_LOG_VERBOSE(
          "Reseting OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->set_is_optimizing(tf);
    IMP_LOG_VERBOSE( "." << std::flush);
  }
  IMP_LOG_VERBOSE( "done." << std::endl);
}

double Optimizer::optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  if (!model_) {
    IMP_THROW("Must give the optimizer a model to optimize",
              base::ValueException);
  }
  set_was_used(true);
  set_is_optimizing_states(true);
  double ret;
  IMP_THREADS((ret, max_steps),
              ret= do_optimize(max_steps););
  set_is_optimizing_states(false);
  return ret;
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates);

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o)
{
  os->set_optimizer(o);
}


void Optimizer::set_restraints(const RestraintsTemp &rs) {
  if (rs.empty()) {
    // otherwise the SF can't figure out the model
    IMP_NEW(RestraintSet, rss, (get_model(), 1.0, "dummy restraint set"));
    RestraintsTemp rt(1, rss);
    set_scoring_function(new internal::RestraintsScoringFunction(rt));
  } else {
    set_scoring_function(new internal::RestraintsScoringFunction(rs));
  }
}

void Optimizer::set_scoring_function(ScoringFunctionAdaptor sf) {
  cache_= sf;
}


Restraints Optimizer::get_restraints() const {
  return cache_->create_restraints();
}

IMPKERNEL_END_NAMESPACE
