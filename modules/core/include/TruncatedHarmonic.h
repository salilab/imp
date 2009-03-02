/**
 *  \file TruncatedHarmonic.h    \brief Truncated harmonic.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_TRUNCATED_HARMONIC_H
#define IMPCORE_TRUNCATED_HARMONIC_H

#include "config.h"
#include "internal/truncated_harmonic.h"
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPCORE_BEGIN_NAMESPACE

enum BoundDirection {LOWER, BOTH, UPPER};

//! A function that is harmonic over an interval.
/** This function is harmonic between center and threshold and then
 asymptotically converges to the limit value.

 The function form above the threshold is currently limit-b/(x-o)
 where x is the offset from the center and b,o are constants chosen to
 make the function smooth and continuous.  This form may change
 without notice unless someone tells us it is important that it does
 not.

 \param[in] DIRECTION Whether to be an upper bound, lower bound, or
 both directions.  It should be one of the BoundDirection enum
 values. If it is LOWER, than the function is 0 for all values above
 the passed center.
 \see Harmonic
 \see HarmonicLowerBound
 \see HarmonicUpperBound
 */
template <int DIRECTION>
class TruncatedHarmonic : public UnaryFunction {
public:
  /** \param[in] center The center point for the harmonic.
   \param[in] k The spring constant for the harmonic.
   \param[in] threshold How far the harmonic term extends from the center.
   \param[in] limit The value to which the function converges above the
   threshold.

   \note I don't like having 4 floats on the initializer list, but
   don't really see an alternative. There are a few sanity checks, so
   the order is a bit hard to get wrong.
   */
  TruncatedHarmonic(Float center,
                    Float k, Float threshold,
                    Float limit):
  d_(center, k, threshold, limit){
  }
  /** Set limit to a reasonable value. */
  TruncatedHarmonic(Float center,
                    Float k, Float threshold):
  d_(center, k, threshold, k*square(threshold-center)){
  }


  virtual ~TruncatedHarmonic() {}

  virtual Float evaluate(Float feature) const {
    if ((DIRECTION == LOWER && (feature > d_.c_))
        || (DIRECTION == UPPER && (feature < d_.c_))) return 0;
    return d_.evaluate(feature);
  }

  virtual FloatPair evaluate_with_derivative(Float feature) const {
    if ((DIRECTION == LOWER && (feature > d_.c_))
        || (DIRECTION == UPPER && (feature < d_.c_))) return FloatPair(0,0);
    return d_.evaluate_with_derivative(feature);
  }

  void show(std::ostream &out=std::cout) const {
    out << "TruncatedHarmonic: " << d_ << std::endl;
  }
private:
  internal::TruncatedHarmonicData d_;
};

//! A specialization for the upper bound
typedef TruncatedHarmonic<UPPER> TruncatedHarmonicUpperBound;
//! A specialization for the lower bound
typedef TruncatedHarmonic<LOWER> TruncatedHarmonicLowerBound;
//! A specialization
typedef TruncatedHarmonic<BOTH> TruncatedHarmonicBound;

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRUNCATED_HARMONIC_H */
