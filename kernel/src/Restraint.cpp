/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

Restraint::Restraint(std::string name):
  Object(internal::make_object_name(name, restraint_index++))
{
}


void Restraint::set_model(Model* model)
{
  IMP_USAGE_CHECK(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_,
            ValueException);
  model_=model;
  if (model) set_was_owned(true);
}

double Restraint::evaluate(bool calc_derivs) const {
  IMP_USAGE_CHECK(model_,
                  "The restraint must be added to the model before being "
                  << "evaluated.", UsageException);
  IMP_USAGE_CHECK(get_model()->get_stage()==Model::NOT_EVALUATING,
            "Restraint::evaluate() cannot be called during model evaluation",
            InvalidStateException);
  RestraintsTemp rr(1, const_cast<Restraint*>(this));
  return get_model()->evaluate(rr, calc_derivs);
}

IMP_END_NAMESPACE
