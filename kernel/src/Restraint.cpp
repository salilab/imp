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

IMP_BEGIN_NAMESPACE

Restraint::Restraint()
{
  is_active_ = true; // active by default
  was_owned_=false;
}


//! Destructor
Restraint::~Restraint()
{
  if (!was_owned_) {
    // can't use virtual functions in the destructor
    IMP_WARN("Restraint " << this << " is being destroyed "
             << "without ever having been added to a model."
             << std::endl);
  }
}


void Restraint::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


bool Restraint::get_is_active() const
{
  return is_active_;
}


void Restraint::set_model(Model* model)
{
  IMP_check(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_,
            ValueException);
  model_=model;
  was_owned_=true;
}

IMP_END_NAMESPACE
