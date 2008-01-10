/**
 *  \file HarmonicLowerBound.cpp  \brief Harmonic lower bound functor.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/unary_functors/HarmonicLowerBound.h"
#include "IMP/log.h"

namespace IMP
{

//! Calculate lower-bound harmonic score with respect to the given feature.
/** If the feature is greater than or equal to the mean, the score is zero.
    \param[in] feature Value of feature being tested.
    \return Score
 */
Float HarmonicLowerBound::operator()(Float feature)
{
  if (feature >= mean_) {
    return 0.0;
  } else {
    return Harmonic::operator()(feature);
  }
}


//! Calculate lower-bound harmonic score and derivative for a feature.
/** If the feature is greater than or equal to the mean, the score is zero.
    \param[in] feature Value of feature being tested.
    \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
    \return Score
 */
Float HarmonicLowerBound::operator()(Float feature, Float& deriv)
{
  if (feature >= mean_) {
    deriv = 0.0;
    return 0.0;
  } else {
    return Harmonic::operator()(feature, deriv);
  }
}

}  // namespace IMP
