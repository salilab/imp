/**
 *  \file Cosine.cpp  \brief Cosine function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/unary_functions/Cosine.h"

namespace IMP
{

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

}  // namespace IMP
