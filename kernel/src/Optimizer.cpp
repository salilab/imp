/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"
#include "IMP/OptimizerState.h"
#include "IMP/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <limits>

IMP_BEGIN_NAMESPACE

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
  boost::tie(flattened_restraints_,
             flattened_weights_)
    =IMP::get_restraints_and_weights(RestraintsTemp(restraints_.begin(),
                                                      restraints_.end()));
  return do_optimize(max_steps);
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates, {
                Optimizer::set_optimizer_state_optimizer(obj, this);
                obj->set_was_used(true);
              },{},
              {Optimizer::set_optimizer_state_optimizer(obj, NULL);});

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o)
{
  os->set_optimizer(o);
}


void Optimizer::set_restraints(const RestraintSetsTemp &rs) {
  restraints_=RestraintSets(rs.begin(), rs.end());
}

double Optimizer::evaluate(bool compute_derivatives) const {
  IMP_FUNCTION_LOG;
  if (restraints_.empty()) {
    return get_model()->evaluate(compute_derivatives);
  } else {
    IMP::Floats ret= get_model()->evaluate(flattened_restraints_,
                                           flattened_weights_,
                                           compute_derivatives);
    return std::accumulate(ret.begin(), ret.end(), 0.0);
  }
}


RestraintSets Optimizer::get_restraints() const {
  if (restraints_.empty()) {
    return RestraintSets(1, model_->get_root_restraint_set());
  } else {
    return restraints_;
  }
}

IMP_END_NAMESPACE
