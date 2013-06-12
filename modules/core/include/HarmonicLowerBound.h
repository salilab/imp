/**
 *  \file IMP/core/HarmonicLowerBound.h    \brief Harmonic lower bound function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_LOWER_BOUND_H
#define IMPCORE_HARMONIC_LOWER_BOUND_H

#include <IMP/core/core_config.h>
#include "Harmonic.h"

IMPCORE_BEGIN_NAMESPACE

//! Lower bound harmonic function (non-zero when feature < mean)
/** \see Harmonic
    \see HarmonicUpperBound
    \see TruncatedHarmonicLowerBound
*/
class HarmonicLowerBound : public Harmonic {
 public:
  /** Create with the given mean and the spring constant k */
  HarmonicLowerBound(Float mean, Float k) : Harmonic(mean, k) {}
  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return feature >= Harmonic::get_mean()
        ? 0.0
        : Harmonic::evaluate(feature);
  }
  virtual DerivativePair evaluate_with_derivative(double feature)
      const IMP_OVERRIDE {
    return feature >= Harmonic::get_mean()
        ? DerivativePair(0.0, 0.0)
        : Harmonic::evaluate_with_derivative(feature);
  }
  IMP_OBJECT_METHODS(HarmonicLowerBound);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_HARMONIC_LOWER_BOUND_H */
