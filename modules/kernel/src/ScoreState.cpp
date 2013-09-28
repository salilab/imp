/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base//log.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/container_base.h"
#include "IMP/kernel/input_output.h"
#include "IMP/kernel/internal/utility.h"
#include <IMP/base/statistics.h>
#include <algorithm>
#include <cmath>
#include <limits>

IMPKERNEL_BEGIN_NAMESPACE

ScoreState::ScoreState(std::string name) : ModelObject(name),
                                           update_order_(-1) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(
      2.1, "Use the ScoreState constructor that takes the model and a name.");
}
ScoreState::ScoreState(kernel::Model *m) : ModelObject(m, "ScoreState%1%"),
                                   update_order_(-1) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(
      2.1, "Use the ScoreState constructor that takes the model and a name.");
}
ScoreState::ScoreState(kernel::Model *m,
                       std::string name) : ModelObject(m, name),
                                           update_order_(-1) {
}

void ScoreState::before_evaluate() {
  IMP_OBJECT_LOG;
  base::Timer t(this, "before_evaluate");
  validate_inputs();
  validate_outputs();
  do_before_evaluate();
}

void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  IMP_OBJECT_LOG;
  base::Timer t(this, "after_evaluate");
  validate_inputs();
  validate_outputs();
  do_after_evaluate(da);
}

namespace {
struct CompOrder {
  bool operator()(const ScoreState *a, const ScoreState *b) const {
    return a->get_update_order() < b->get_update_order();
  }
};
}

ScoreStatesTemp get_update_order(ScoreStatesTemp in) {
  IMP_FUNCTION_LOG;
  if (in.empty()) return in;
  std::sort(in.begin(), in.end());
  in.erase(std::unique(in.begin(), in.end()), in.end());
  std::sort(in.begin(), in.end(), CompOrder());
  IMP_LOG_TERSE("Order: " << in << std::endl);
  return in;
}

void ScoreState::handle_set_has_required_score_states(bool tf) {
  if (tf) {
    IMP_USAGE_CHECK(update_order_ == -1,
                    "Already had update order");
    if (!get_model()->do_get_required_score_states(this).empty()) {
      update_order_ = get_required_score_states().back()->get_update_order()
        + 1;
      IMP_LOG_VERBOSE("Update order for " << get_name()
                      << " is " << update_order_ << " due to "
     << get_model()->do_get_required_score_states(this).back()->get_name()
                      << std::endl);
    } else {
      update_order_ = 0;
    }
  } else {
    update_order_ = -1;
  }
}

ParticlesTemp ScoreState::get_input_particles() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_inputs() instead");
  return IMP::kernel::get_input_particles(get_inputs());
}
ContainersTemp ScoreState::get_input_containers() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_inputs() instead");
  return IMP::kernel::get_input_containers(get_inputs());
}
ParticlesTemp ScoreState::get_output_particles() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_outputs() instead");
  return IMP::kernel::get_output_particles(get_outputs());
}
ContainersTemp ScoreState::get_output_containers() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_outputs() instead");
  return IMP::kernel::get_output_containers(get_outputs());
}

IMPKERNEL_END_NAMESPACE
