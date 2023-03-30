/**
 *  \file IMP/core/HarmonicUpperBound.h    \brief Harmonic upper bound function.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_UPPER_BOUND_H
#define IMPCORE_HARMONIC_UPPER_BOUND_H

#include <IMP/core/core_config.h>
#include "Harmonic.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Upper bound harmonic function (non-zero when feature > mean)
/** \see Harmonic
    \see HarmonicLowerBound
    \see TruncatedHarmonicUpperBound
 */
class HarmonicUpperBound : public Harmonic {
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Harmonic>(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(HarmonicUpperBound);
 public:
  /** Create with the given mean and the spring constant k */
  HarmonicUpperBound(Float mean, Float k) : Harmonic(mean, k) {}
  HarmonicUpperBound() {}
  virtual double evaluate(double feature) const override {
    return feature <= Harmonic::get_mean() ? 0.0 : Harmonic::evaluate(feature);
  }
  virtual DerivativePair evaluate_with_derivative(double feature) const
      override {
    return feature <= Harmonic::get_mean()
               ? DerivativePair(0.0, 0.0)
               : Harmonic::evaluate_with_derivative(feature);
  }
  IMP_OBJECT_METHODS(HarmonicUpperBound);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_HARMONIC_UPPER_BOUND_H */
