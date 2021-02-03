/**
 *  \file IMP/core/TruncatedHarmonic.h    \brief Truncated harmonic.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TRUNCATED_HARMONIC_H
#define IMPCORE_TRUNCATED_HARMONIC_H

#include <IMP/core/core_config.h>
#include "internal/truncated_harmonic.h"
#include <IMP/UnaryFunction.h>
#include <IMP/object_macros.h>
#include <IMP/utility.h>

IMPCORE_BEGIN_NAMESPACE

enum BoundDirection {
  LOWER,
  BOTH,
  UPPER
};

//! A function that is harmonic over an interval.
/** This function is harmonic between center and threshold and then
 asymptotically converges to the limit value.

 The function form above the threshold is currently limit-b/(x-o)
 where x is the offset from the center and b,o are constants chosen to
 make the function smooth and continuous.  This form may change
 without notice unless someone tells us it is important that it does
 not.

 \param[in] DIRECTION Whether the harmonic is of an upper bound, lower bound, or
 both directions type.  It should be one of the BoundDirection enum
 values. If it is LOWER, than the function is 0 for all values above
 the passed center.
 \see Harmonic
 \see HarmonicLowerBound
 \see HarmonicUpperBound
 */
template <int DIRECTION>
class TruncatedHarmonic : public UnaryFunction {
 public:
  /** \param[in] center The center point for the truncated harmonic.
   \param[in] k The spring constant for the truncated harmonic.
   \param[in] threshold How far the harmonic term extends from the center.
   \param[in] limit The value to which the function converges above the
   threshold.

   \note I don't like having 4 floats on the initializer list, but
   don't really see an alternative. There are a few sanity checks, so
   the order is a bit hard to get wrong.
   */
  TruncatedHarmonic(Float center, Float k, Float threshold, Float limit)
      : d_(center, k, threshold, limit) {}
  /** Same as other constructor, but automatically set limit to a reasonable default value. */
  TruncatedHarmonic(Float center, Float k, Float threshold)
      : d_(center, k, threshold, k * square(threshold)) {}
  virtual DerivativePair evaluate_with_derivative(double feature) const
      IMP_OVERRIDE {
    return DerivativePair(evaluate(feature),
                          ((DIRECTION == LOWER && (feature > d_.c_)) ||
                           (DIRECTION == UPPER && (feature < d_.c_)))
                              ? 0
                              : d_.evaluate_with_derivative(feature).second);
  }
  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return ((DIRECTION == LOWER && (feature > d_.c_)) ||
            (DIRECTION == UPPER && (feature < d_.c_)))
               ? 0
               : d_.evaluate(feature);
  }
  IMP_OBJECT_METHODS(TruncatedHarmonic);

 private:
  internal::TruncatedHarmonicData d_;
};

//! A specialization of TruncatedHarmonic that may be non-zero only
//! above the center value (always zero below it)
typedef TruncatedHarmonic<UPPER> TruncatedHarmonicUpperBound;

//! A specialization of TruncatedHarmonic that may be non-zero only
//! below the center value (always zero above it)
typedef TruncatedHarmonic<LOWER> TruncatedHarmonicLowerBound;

//! A specialization of TruncatedHarmonic that may be non-zero in both
//! directions, below or above the center value
typedef TruncatedHarmonic<BOTH> TruncatedHarmonicBound;

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TRUNCATED_HARMONIC_H */
