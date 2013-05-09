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
  IMP_UNARY_FUNCTION_INLINE(
      HarmonicLowerBound,
      feature >= Harmonic::get_mean() ? 0.0 : Harmonic::evaluate(feature),
      feature >= Harmonic::get_mean()
          ? 0.0
          : Harmonic::evaluate_with_derivative(feature).second,
      "HarmonicLB: " << Harmonic::get_mean() << " and " << Harmonic::get_k()
                     << std::endl);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_HARMONIC_LOWER_BOUND_H */
