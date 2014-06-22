/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
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

static const std::string str_before_evaluate("before_evaluate");
static const std::string str_after_evaluate("after_evaluate");

ScoreState::ScoreState(kernel::Model *m, std::string name)
    : ModelObject(m, name), update_order_(-1) {}

void ScoreState::before_evaluate() {
  IMP_OBJECT_LOG;
  base::Timer t(this, str_before_evaluate);
  validate_inputs();
  validate_outputs();
  do_before_evaluate();
}

void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  IMP_OBJECT_LOG;
  base::Timer t(this, str_after_evaluate);
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
    IMP_USAGE_CHECK(update_order_ == -1, "Already had update order");
    if (!get_model()->do_get_required_score_states(this).empty()) {
      update_order_ =
          get_required_score_states().back()->get_update_order() + 1;
      IMP_LOG_VERBOSE(
          "Update order for "
          << get_name() << " is " << update_order_ << " due to "
          << get_model()->do_get_required_score_states(this).back()->get_name()
          << std::endl);
    } else {
      update_order_ = 0;
    }
  } else {
    update_order_ = -1;
  }
}

IMPKERNEL_END_NAMESPACE
