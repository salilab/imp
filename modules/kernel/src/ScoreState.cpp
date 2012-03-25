/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/log.h"
#include "IMP/ScoreState.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"
#include <algorithm>
#include <cmath>
#include <limits>

IMP_BEGIN_NAMESPACE

ScoreState::ScoreState(std::string name) :
  Object(name)
{
  order_=-1;
}


void ScoreState::before_evaluate() {
  do_before_evaluate();
}


void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  do_after_evaluate(da);
}

//! Give accesss to model particle data.
/** \param[in] model_data All particle data in the model.
 */
void ScoreState::set_model(Model* model)
{
  IMP_USAGE_CHECK(!model || !model_
            || (model_ && model_ == model),
            "Model* different from already stored model "
                  << model->get_name() << " " << model_->get_name());
  model_ = model;
  set_was_used(true);
}

namespace {
struct CompOrder {
  bool operator()(const ScoreState*a,
                  const ScoreState*b) const {
    return a->order_ < b->order_;
  }
};
}


ScoreStatesTemp get_ordered_score_states( ScoreStatesTemp in) {
  if (in.empty()) return in;
  // make sure the order_ entries are up to date
  if (!in[0]->get_model()->get_has_dependencies()) {
    in[0]->get_model()->compute_dependencies();
  }
  std::sort(in.begin(), in.end(), CompOrder());
  return in;
}

IMP_END_NAMESPACE
