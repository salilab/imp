/**
 *  \file HarmonicUpperBound.h    \brief Harmonic upper bound function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_HARMONIC_UPPER_BOUND_H
#define __IMP_HARMONIC_UPPER_BOUND_H

#include "Harmonic.h"

namespace IMP
{

//! Upper bound harmonic function (non-zero when feature > mean)
/** \ingroup unaryf
 */
class HarmonicUpperBound : public Harmonic
{
public:
  HarmonicUpperBound(Float mean, Float k) : Harmonic(mean, k) {}
  virtual ~HarmonicUpperBound() {}

  //! Calculate upper-bound harmonic score with respect to the given feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float evaluate(Float feature) const {
    if (feature <= Harmonic::get_mean()) {
      return 0.0;
    } else {
      return Harmonic::evaluate(feature);
    }
  }

  //! Calculate upper-bound harmonic score and derivative for a feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual FloatPair evaluate_with_derivative(Float feature) const {
    if (feature <= Harmonic::get_mean()) {
      return std::make_pair(0.0f, 0.0f);
    } else {
      return Harmonic::evaluate_with_derivative(feature);
    }
  }

  void show(std::ostream &out=std::cout) const {
    out << "HarmonicUB: " << Harmonic::get_mean() 
        << " and " << Harmonic::get_k() << std::endl;
  }
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_UPPER_BOUND_H */
