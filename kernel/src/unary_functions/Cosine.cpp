/**
 *  \file Cosine.cpp  \brief Cosine function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/unary_functions/Cosine.h"
#include "../mystdexcept.h"

namespace IMP
{

Float Cosine::operator()(Float feature)
{
  return std::abs(force_constant_)
         - force_constant_ * std::cos(periodicity_ * feature + phase_);
}

Float Cosine::operator()(Float feature, Float& deriv)
{
  deriv = force_constant_ * periodicity_
          * std::sin(periodicity_ * feature + phase_);
  return operator()(feature);
}

}  // namespace IMP
