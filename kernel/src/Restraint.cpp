/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int restraint_index=0;
}

Restraint::Restraint(std::string name)
{
  set_name(internal::make_object_name(name, restraint_index++));
}


void Restraint::set_model(Model* model)
{
  IMP_check(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_,
            ValueException);
  model_=model;
  set_was_owned(true);
}

double Restraint::evaluate(DerivativeAccumulator *accum) const {
  IMP_check(get_model()->get_stage()== Model::EVALUATE
            || get_model()->get_stage()==Model::NOT_EVALUATING,
            "Restraint::evaluate() cannot be called during model evaluation",
            InvalidStateException);
  bool not_eval=get_model()->get_stage() != Model::EVALUATE;
  if (not_eval) {
    get_model()->before_evaluate();
  }
  double ret=unprotected_evaluate(accum);
  if (not_eval) {
    get_model()->after_evaluate(accum);
    get_model()->cur_stage_=Model::NOT_EVALUATING;
  }
  return ret;
}

double Restraint::incremental_evaluate(DerivativeAccumulator *accum) const {
  IMP_check(get_model()->get_stage()== Model::EVALUATE
            || get_model()->get_stage()==Model::NOT_EVALUATING,
            "Restraint::evaluate() cannot be called during model evaluation",
            InvalidStateException);
  IMP_check(get_model()->get_stage()== Model::EVALUATE
            || !accum, "Evaluation with derivatives is not supported.",
            ValueException);
  bool not_eval=get_model()->get_stage() != Model::EVALUATE;
  if (not_eval) {
    get_model()->before_evaluate();
  }
  double ret=unprotected_incremental_evaluate(accum);
  if (not_eval) {
    get_model()->after_evaluate(accum);
    get_model()->cur_stage_=Model::NOT_EVALUATING;
  }
  return ret;
}


IMP_END_NAMESPACE
