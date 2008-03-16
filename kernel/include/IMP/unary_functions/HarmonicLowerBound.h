/**
 *  \file HarmonicLowerBound.h    \brief Harmonic lower bound function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_HARMONIC_LOWER_BOUND_H
#define __IMP_HARMONIC_LOWER_BOUND_H

#include "Harmonic.h"

namespace IMP
{

//! Lower bound harmonic function (non-zero when feature < mean)
/** \ingroup unaryf
 */
class HarmonicLowerBound : public Harmonic
{
public:
  HarmonicLowerBound(Float mean, Float k) : Harmonic(mean, k) {}
  virtual ~HarmonicLowerBound() {}

  //! Calculate lower-bound harmonic score with respect to the given feature.
  /** If the feature is greater than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \return Score
  */
  virtual Float evaluate(Float feature) {
    if (feature >= Harmonic::get_mean()) {
      return 0.0;
    } else {
      return Harmonic::evaluate(feature);
    }
  }

  //! Calculate lower-bound harmonic score and derivative for a feature.
  /** If the feature is greater than or equal to the mean, the score is zero.
      \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \return Score
   */
  virtual Float evaluate_deriv(Float feature, Float& deriv) {
    if (feature >= Harmonic::get_mean()) {
      deriv = 0.0;
      return 0.0;
    } else {
      return Harmonic::evaluate_deriv(feature, deriv);
    }
  }

  void show(std::ostream &out=std::cout) const {
    out << "HarmonicLB: " << Harmonic::get_mean() 
        << " and " << Harmonic::get_k() << std::endl;
  }
};

} // namespace IMP

#endif  /* __IMP_HARMONIC_LOWER_BOUND_H */
