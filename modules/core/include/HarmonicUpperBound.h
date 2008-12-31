/**
 *  \file HarmonicUpperBound.h    \brief Harmonic upper bound function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_UPPER_BOUND_H
#define IMPCORE_HARMONIC_UPPER_BOUND_H

#include "config.h"
#include "Harmonic.h"

IMPCORE_BEGIN_NAMESPACE

//! Upper bound harmonic function (non-zero when feature > mean)
/** */
class HarmonicUpperBound : public Harmonic
{
public:
  /** Create with the given mean and the spring constant k */
  HarmonicUpperBound(Float mean, Float k) : Harmonic(mean, k) {}
  virtual ~HarmonicUpperBound() {}

  virtual Float evaluate(Float feature) const {
    if (feature <= Harmonic::get_mean()) {
      return 0.0;
    } else {
      return Harmonic::evaluate(feature);
    }
  }

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

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HARMONIC_UPPER_BOUND_H */
