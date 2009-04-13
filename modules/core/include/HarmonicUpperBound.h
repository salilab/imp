/**
 *  \file HarmonicUpperBound.h    \brief Harmonic upper bound function.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_UPPER_BOUND_H
#define IMPCORE_HARMONIC_UPPER_BOUND_H

#include "config.h"
#include "Harmonic.h"

IMPCORE_BEGIN_NAMESPACE

//! Upper bound harmonic function (non-zero when feature > mean)
/** \see Harmonic
    \see HarmonicLowerBound
    \see TruncatedHarmonicUpperBound
 */
class HarmonicUpperBound : public Harmonic
{
public:
  /** Create with the given mean and the spring constant k */
  HarmonicUpperBound(Float mean, Float k) : Harmonic(mean, k) {}
  IMP_UNARY_FUNCTION_INLINE(HarmonicUpperBound,
                            internal::version_info,
                            feature <= Harmonic::get_mean() ?
                            0.0: Harmonic::evaluate(feature),
                            feature <= Harmonic::get_mean() ?
                            0.0:
                            Harmonic::evaluate_with_derivative(feature).second,
                            "HarmonicUB: " << Harmonic::get_mean()
                            << " and " << Harmonic::get_k() << std::endl);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HARMONIC_UPPER_BOUND_H */
