/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/log.h>
#include "IMP/kernel/Optimizer.h"
#include "IMP/kernel/OptimizerState.h"
#include "IMP/kernel/RestraintSet.h"
#include "IMP/kernel/generic.h"
#include "IMP/kernel/dependency_graph.h"
#include "IMP/kernel/internal/graph_utility.h"
#include "IMP/kernel/internal/RestraintsScoringFunction.h"
#include "IMP/kernel/internal/container_helpers.h"
#include <IMP/base/thread_macros.h>
#include "IMP/kernel/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <limits>
#include <algorithm>

IMPKERNEL_BEGIN_NAMESPACE

Optimizer::Optimizer(kernel::Model *m, std::string name)
    : ModelObject(m, name) {
  set_was_used(true);
  stop_on_good_score_ = false;
}

Optimizer::~Optimizer() {}

void Optimizer::update_states() const {
  IMP_LOG_VERBOSE("Updating OptimizerStates " << std::flush);
  IMP_FOREACH(ScoreState * ss, get_required_score_states()) {
    ss->before_evaluate();
  }
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG_VERBOSE("." << std::flush);
  }
  IMP_LOG_VERBOSE("done." << std::endl);
}

void Optimizer::set_is_optimizing_states(bool tf) const {
  IMP_LOG_VERBOSE("Reseting OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->set_is_optimizing(tf);
    IMP_LOG_VERBOSE("." << std::flush);
  }
  IMP_LOG_VERBOSE("done." << std::endl);
}

double Optimizer::optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  set_has_required_score_states(true);
  set_was_used(true);
  set_is_optimizing_states(true);
  double ret;
  IMP_THREADS((ret, max_steps), ret = do_optimize(max_steps););
  set_is_optimizing_states(false);
  return ret;
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state, OptimizerState *,
              OptimizerStates);

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os,
                                              Optimizer *o) {
  os->set_was_used(true);
  os->set_optimizer(o);
}

void Optimizer::set_scoring_function(ScoringFunctionAdaptor sf) {
  scoring_function_ = sf;
}

ModelObjectsTemp Optimizer::get_optimizer_state_inputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i = 0; i < get_number_of_optimizer_states(); ++i) {
    ret += get_optimizer_state(i)->get_inputs();
  }
  return ret;
}

IMPKERNEL_END_NAMESPACE
