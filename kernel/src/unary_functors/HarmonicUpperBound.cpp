/**
 *  \file HarmonicUpperBound.cpp  \brief Harmonic upper bound functor.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/unary_functors/HarmonicUpperBound.h"
#include "IMP/log.h"

namespace IMP
{

//! Calculate upper-bound harmonic score with respect to the given feature.
/** If the feature is less than or equal to the mean, the score is zero.
    \param[in] feature Value of feature being tested.
    \return Score
 */
Float HarmonicUpperBound::operator()(Float feature)
{
  if (feature <= mean_) {
    return 0.0;
  } else {
    return Harmonic::operator()(feature);
  }
}


//! Calculate upper-bound harmonic score and derivative for a feature.
/** If the feature is less than or equal to the mean, the score is zero.
    \param[in] feature Value of feature being tested.
    \param[out] deriv Partial derivative of the score with respect to
                      the feature value.
    \return Score
 */
Float HarmonicUpperBound::operator()(Float feature, Float& deriv)
{
  if (feature <= mean_) {
    deriv = 0.0;
    return 0.0;
  } else {
    return Harmonic::operator()(feature, deriv);
  }
}

}  // namespace IMP
