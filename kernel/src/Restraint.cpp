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

IMP_END_NAMESPACE
