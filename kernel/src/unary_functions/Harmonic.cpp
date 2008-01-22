/**
 *  \file Harmonic.cpp  \brief Harmonic function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/unary_functions/Harmonic.h"
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

  // convert standard deviation to force constant, by dividing by sqrt(RT/2)
  // where T=297.15K and energy units are kcal/mol
  sd = sd_ / 0.54318464;
  e = (feature - mean_) / sd;
  deriv = e / sd * 2.0;
  return e * e;
}

}  // namespace IMP
