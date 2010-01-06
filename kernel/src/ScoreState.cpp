/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

ScoreState::ScoreState(std::string name) :
  Object(internal::make_object_name(name, scorestate_index++))
{
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
