/**
 *  \file Harmonic.cpp  \brief Harmonic functor.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/unary_functors/Harmonic.h"
#include "IMP/log.h"

namespace IMP
{

//! Calculate harmonic score with respect to the given feature.
/** \param[in] feature Value of feature being tested.
    \return Score
 */
Float Harmonic::operator()(Float feature)
{
  Float dummy;

  return operator()(feature, dummy);
}

//! Calculate harmonic score and derivative with respect to the given feature.
/** \param[in] feature Value of feature being tested.
    \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
    \return Score
 */
Float Harmonic::operator()(Float feature, Float& deriv)
{
  Float e;
  Float sd;

  sd = sd_ / 0.54318463; // correction factor to match Modeller
  e = (feature - mean_) / sd;
  deriv = e / sd * 2.0; // * 2.0 is correction factor to match Modeller
  return e * e;
}

}  // namespace IMP
