/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

Optimizer::Optimizer() : Object("Optimizer%1%") {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1,
                                    "Use the constructor that takes a model.");
  set_was_used(true);
  min_score_ = -std::numeric_limits<double>::max();
  stop_on_good_score_ = false;
}

Optimizer::Optimizer(Model *m, std::string name) : Object(name) {
  if (m) set_model(m);
  set_was_used(true);
  min_score_ = -std::numeric_limits<double>::max();
  stop_on_good_score_ = false;
}

Optimizer::~Optimizer() {}

void Optimizer::set_model(Model *m) {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1,
                                    "Use the constructor that takes a model.");
  cache_ = m->create_model_scoring_function();
  cache_->set_was_used(true);
  model_ = m;
}

void Optimizer::update_states() const {
  IMP_LOG_VERBOSE("Updating OptimizerStates " << std::flush);
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
  if (!model_) {
    IMP_THROW("Must give the optimizer a model to optimize",
              base::ValueException);
  }
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
  os->set_optimizer(o);
}

void Optimizer::set_restraints(const RestraintsTemp &rs) {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1,
                                "Use Optimizer::set_scoring_function instead");
  if (rs.empty()) {
    // otherwise the SF can't figure out the model
    IMP_NEW(RestraintSet, rss, (get_model(), 1.0, "dummy restraint set"));
    RestraintsTemp rt(1, rss);
    set_scoring_function(new internal::RestraintsScoringFunction(rt));
  } else {
    set_scoring_function(new internal::RestraintsScoringFunction(rs));
  }
}

void Optimizer::set_scoring_function(ScoringFunctionAdaptor sf) { cache_ = sf; }

Restraints Optimizer::get_restraints() const {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1,
                                "Use Optimizer::get_scoring_function instead");
  return cache_->create_restraints();
}









  FloatIndexes Optimizer::get_optimized_attributes() const {
    return get_model()->get_optimized_attributes();
  }

  void Optimizer::set_value(FloatIndex fi, double v) const {
    get_model()->set_attribute(fi.get_key(), fi.get_particle(), v);
  }

 Float Optimizer::get_value(FloatIndex fi) const {
    return get_model()->get_attribute(fi.get_key(), fi.get_particle());
  }

  Float Optimizer::get_derivative(FloatIndex fi) const {
    return get_model()->get_derivative(fi.get_key(), fi.get_particle());
  }

  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double Optimizer::width(FloatKey k) const {
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Use get_width instead");
    return get_width(k);
  }

  double Optimizer::get_width(FloatKey k) const {
    if (widths_.size() <= k.get_index() || widths_[k.get_index()] == 0) {
      FloatRange w = model_->get_range(k);
      double wid = static_cast<double>(w.second) - w.first;
      widths_.resize(std::max(widths_.size(), size_t(k.get_index() + 1)), 0.0);
      if (wid > .0001) {
        //double nwid= std::pow(2, std::ceil(log2(wid)));
        widths_[k.get_index()] = wid;
      } else {
        widths_[k.get_index()] = 1.0;
      }
    }
    return widths_[k.get_index()];
  }

  void Optimizer::set_scaled_value (FloatIndex fi, Float v) const {
    double wid = get_width(fi.get_key());
    set_value(fi, v * wid);
  }

  double Optimizer::get_scaled_value (FloatIndex fi) const {
    double uv = get_value(fi);
    double wid = get_width(fi.get_key());
    return uv / wid;
  }

  double Optimizer::get_scaled_derivative(FloatIndex fi) const {
    double uv = get_derivative(fi);
    double wid = get_width(fi.get_key());
    return uv * wid;
  }

  void Optimizer::clear_range_cache() {
    widths_.clear();
  }
IMPKERNEL_END_NAMESPACE
