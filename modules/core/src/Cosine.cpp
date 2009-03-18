/**
 *  \file Cosine.cpp  \brief Cosine function.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/Cosine.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

Float Cosine::evaluate(Float feature) const
{
  return std::abs(force_constant_)
         - force_constant_ * std::cos(periodicity_ * feature + phase_);
}

FloatPair Cosine::evaluate_with_derivative(Float feature) const
{
  Float deriv = force_constant_ * periodicity_
    * std::sin(periodicity_ * feature + phase_);
  return std::make_pair(evaluate(feature), deriv);
}

IMPCORE_END_NAMESPACE
