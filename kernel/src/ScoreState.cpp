/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/ScoreState.h"
#include "IMP/internal/utility.h"
#include <cmath>
#include <limits>

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int scorestate_index=0;
}

ScoreState::ScoreState(std::string name) : name_(name)
{
  update_iteration_= std::numeric_limits<unsigned int>::max();
  after_iteration_= std::numeric_limits<unsigned int>::max();
  set_name(internal::make_object_name(name, scorestate_index++));
}


void ScoreState::before_evaluate(unsigned int iter) {
  if (update_iteration_ != iter) {
    update_iteration_= iter;
    do_before_evaluate();
  }
}


void ScoreState::after_evaluate(unsigned int iter,
                                DerivativeAccumulator *da) {
  if (after_iteration_ != iter) {
    after_iteration_= iter;
    do_after_evaluate(da);
  }
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
  IMP_USAGE_CHECK(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_,
            ValueException);
  model_ = model;
  set_was_owned(true);
}

IMP_END_NAMESPACE
