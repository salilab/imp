/**
 *  \file IMP/core/HarmonicWell.h    \brief Harmonic function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_WELL_H
#define IMPCORE_HARMONIC_WELL_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPCORE_BEGIN_NAMESPACE

//! A well with harmonic barriers
/** The well is defined by a center, a width and a k. The score is
    0 if the feature is within the width/2.0. Outside of the width
    the score is a harmonic.
    \see TruncatedHarmonic
    \see Harmonic
    \see HarmonicUpperBound
    \see HarmonicLowerBound
 */
class HarmonicWell : public UnaryFunction
{
  double get_score(double x) const {
    if (x < lb_) return .5*k_*square(x-lb_);
    else if (x > ub_) return .5*k_*square(x-ub_);
    else return 0;
  }
  double get_derivative(double x) const {
    if (x < lb_) return k_*(x-lb_);
    else if (x > ub_) return k_*(x-ub_);
    else return 0;
  }
public:
  //! Initialize with the lower and upper bounds and the spring constant
  HarmonicWell(const FloatRange& well, double k) : lb_(well.first),
                                                      ub_(well.second),
                                                      k_(k) {
    IMP_USAGE_CHECK(well.first <= well.second,
                    "The width should be non-negative");
    IMP_USAGE_CHECK(k >=0, "The k should be non-negative");
  }

  virtual DerivativePair evaluate_with_derivative(double feature) const {
    return DerivativePair(get_score(feature), get_derivative(feature));
  }

  virtual double evaluate(double feature) const { return get_score(feature); }

  IMP_OBJECT_METHODS(HarmonicWell);

private:
  double lb_, ub_, k_;
};

IMP_OBJECTS(HarmonicWell, HarmonicWells);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HARMONIC_WELL_H */
