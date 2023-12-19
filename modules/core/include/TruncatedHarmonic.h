/**
 *  \file IMP/core/TruncatedHarmonic.h    \brief Truncated harmonic.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
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

//! A function that is harmonic over an bounded interval.
/** This is a harmonic function of the form 
    $f(x) = 0.5 \cdot k \cdot (x-center)^2$ within a bounded interval
    around a center value, in the bounded interval where $\|x-center\|<threshold$,
    i.e. up to a threshold offset from the center. Beyond the threshold offset,
    the function asymptotically converges to the limit value. 

    \param[in] DIRECTION This template parameter determines whether the function 
    is non-zero only for input values that are greater than the center (UPPER), 
    lower than the center (LOWER), or both (BOTH). In Python code, these choices
    can be specified using the types TruncatedHarmonicUpperBound, 
    TruncatedHarmonicLowerBound, and TruncatedHarmonic, respectively.

    For example, if the center equals to 5, the threshold to 2, and the direction 
    is BOTH (see below), then the function is harmonic in the interval [3,7], 
    and beyond that interval it converges to the limit value in either direction.  
    If the direction is LOWER, then the function is  harmonic in the interval
    [3,5], it converges to the limit value for values lower thans 3, and is 0
    for values higher than 5. If it is UPPER, then the function is harmonic in 
    the interval [5,7], it is 0 for values lower than 5, and it converges to 
    the limit value for values higher than 7.

    @note The function form beyond the threshold offset from center is currently 
    limit-b/(x-o) where x is the offset from the center and b,o are constants 
    chosen to make the function smooth and continuous.  This form may change
    without notice unless someone tells us it is important that it does
    not.

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
      override {
    return DerivativePair(evaluate(feature),
                          ((DIRECTION == LOWER && (feature > d_.c_)) ||
                           (DIRECTION == UPPER && (feature < d_.c_)))
                              ? 0
                              : d_.evaluate_with_derivative(feature).second);
  }
  virtual double evaluate(double feature) const override {
    return ((DIRECTION == LOWER && (feature > d_.c_)) ||
            (DIRECTION == UPPER && (feature < d_.c_)))
               ? 0
               : d_.evaluate(feature);
  }
  IMP_OBJECT_METHODS(TruncatedHarmonic);

 private:
  internal::TruncatedHarmonicData d_;
};

//! A specialization of TruncatedHarmonic that returns a non-zero value 
//! only for input values that are greater than the center value
typedef TruncatedHarmonic<UPPER> TruncatedHarmonicUpperBound;

//! A specialization of TruncatedHarmonic that returns a non-zero value 
//! only for input values that are lower than the center value
typedef TruncatedHarmonic<LOWER> TruncatedHarmonicLowerBound;

//! A specialization of TruncatedHarmonic that returns non-zero value 
//! for any input values other than center value 
typedef TruncatedHarmonic<BOTH> TruncatedHarmonicBound;

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TRUNCATED_HARMONIC_H */
