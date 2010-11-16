/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/internal/utility.h"
#include <numeric>

IMP_BEGIN_NAMESPACE

Restraint::Restraint(std::string name):
  Object(name)
{
}


void Restraint::set_model(Model* model)
{
  IMP_USAGE_CHECK(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_);
  model_=model;
  if (model) set_was_used(true);
}

double Restraint::evaluate(bool calc_derivs) const {
  IMP_USAGE_CHECK(model_,
                  "The restraint must be added to the model before being "
                  << "evaluated.");
  IMP_USAGE_CHECK(get_model()->get_stage()==Model::NOT_EVALUATING,
            "Restraint::evaluate() cannot be called during model evaluation");
  RestraintsTemp rr(1, const_cast<Restraint*>(this));
  std::vector<double> ws(1, 1.0);
  Floats v= get_model()->evaluate(rr, ws, calc_derivs);
  return std::accumulate(v.begin(), v.end(), 0.0);
}

IMP_END_NAMESPACE
