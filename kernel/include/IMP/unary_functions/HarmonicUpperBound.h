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
class IMPDLLEXPORT HarmonicUpperBound : public Harmonic
{
public:
  HarmonicUpperBound(Float mean, Float sd) : Harmonic(mean, sd) {}
  virtual ~HarmonicUpperBound() {}

  //! Calculate upper-bound harmonic score with respect to the given feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float operator()(Float feature);

  //! Calculate upper-bound harmonic score and derivative for a feature.
  /** If the feature is less than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float operator()(Float feature, Float& deriv);

  void show(std::ostream &out=std::cout) const {
    out << "HarmonicLUB: " << mean_ << " and " << sd_ << std::endl;
  }
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_UPPER_BOUND_H */
