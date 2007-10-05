/*
 *  Score_Func.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Score_Func.h"

namespace imp
{

/**
  Calculate harmonic score with respect to the given feature.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic::operator()(Float feature, Float mean, Float sd)
{
  return harmonic(feature, mean, sd);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic::operator()(Float feature, Float mean, Float sd, Float& deriv)
{
  return harmonic(feature, mean, sd, deriv);
}

/**
  Calculate harmonic score with respect to the given feature.
  Implemented to support lower and upper bound harmonics as well.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic::harmonic(Float feature, Float mean, Float sd)
{
  Float dummy;

  return harmonic(feature, mean, sd, dummy);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature. Implemented to support lower
  and upper bound harmonics as well.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic::harmonic(Float feature, Float mean, Float sd, Float& deriv)
{
  Float e;

  sd /= 0.54318463; // correction factor to match Modeller
  e = (feature - mean) / sd;
  deriv = e / sd * 2.0; // * 2.0 is correction factor to match Modeller
  return e * e;
}

/**
  Calculate harmonic score with respect to the given feature if
  the feature is less than the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic_Lower_Bound::operator()(Float feature, Float mean, Float sd)
{
  if (feature >= mean)
    return 0.0;
  else
    return harmonic(feature, mean, sd);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature if the feature is less than
  the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic_Lower_Bound::operator()(Float feature, Float mean, Float sd, Float& deriv)
{
  if (feature >= mean) {
    deriv = 0.0;
    return 0.0;
  } else
    return harmonic(feature, mean, sd, deriv);
}

/**
  Calculate harmonic score with respect to the given feature if
  the feature is greater than the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.
 */

Float Harmonic_Upper_Bound::operator()(Float feature, Float mean, Float sd)
{
  if (feature <= mean)
    return 0.0;
  else
    return harmonic(feature, mean, sd);
}

/**
  Calculate harmonic score and partial derivative of the score
  with respect to the given feature if the feature is greater than
  the mean. Otherwise, the score is zero.

  \param[in] feature Value of feature being tested.

  \param[in] mean Expected mean value of the feature.

  \param[in] sd Allowable standard deviation value of the feature.

  \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
 */

Float Harmonic_Upper_Bound::operator()(Float feature, Float mean, Float sd, Float& deriv)
{
  if (feature <= mean) {
    deriv = 0.0;
    return 0.0;
  } else
    return harmonic(feature, mean, sd, deriv);
}

}  // namespace imp

