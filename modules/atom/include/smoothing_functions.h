/**
 *  \file smoothing_functions.h    Classes to smooth nonbonded interactions
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_SMOOTHING_FUNCTIONS_H
#define IMPATOM_SMOOTHING_FUNCTIONS_H

#include "config.h"

#include <IMP/base_types.h>
#include <IMP/Object.h>

IMPATOM_BEGIN_NAMESPACE

//! Class for smoothing nonbonded interactions as a function of distance.
/** The functors are given the score (and optionally its first derivative)
    at a given distance (typically from a CoulombPairScore) and return
    a smoothed form of the score. Such functors are used to avoid a
    discontinuity in the scoring function at the cutoff distance.
    Smoothing functions usually offset the score by a constant value
    (a shift function) or smooth it from its normal value to zero over
    a defined range (a switch function).
 */
class IMPATOMEXPORT SmoothingFunction : public Object
{
public:
  SmoothingFunction();

  //! Smooth the score at a given distance.
  virtual double operator()(double score, double distance) const = 0;

  //! Smooth the score and its first derivative at a given distance.
  virtual DerivativePair operator()(double score, double deriv,
                                    double distance) const = 0;

  IMP_REF_COUNTED_DESTRUCTOR(SmoothingFunction)
};


//! Smooth interaction scores by switching the derivatives (force switch)
/** This function leaves the scores unaffected for distances below or equal
    to min_distance, and then switches the derivatives smoothly to zero between
    min_distance and max_distance. This is roughly equivalent to CHARMM's
    force switch nonbonded interaction smoothing.
 */
class IMPATOMEXPORT ForceSwitch : public SmoothingFunction
{
  double min_distance_, max_distance_;
  double value_prefactor_, deriv_prefactor_;

  inline double get_value(double distance) const {
    if (distance <= min_distance_) {
      return 1.0;
    } else if (distance > max_distance_) {
      return 0.0;
    } else {
      double d = max_distance_ - distance;
      return value_prefactor_ * d * d * (max_distance_ + 2.0 * distance
                                      - 3.0 * min_distance_);
    }
  }

  inline double get_deriv(double distance) const {
    if (distance <= min_distance_ || distance > max_distance_) {
      return 0.0;
    } else {
      return deriv_prefactor_ * (max_distance_ - distance)
             * (min_distance_ - distance);
    }
  }

public:
  ForceSwitch(double min_distance, double max_distance)
             : min_distance_(min_distance), max_distance_(max_distance) {
    double dist_dif = max_distance - min_distance;
    value_prefactor_ = 1.0 / (dist_dif * dist_dif * dist_dif);
    deriv_prefactor_ = 6.0 * value_prefactor_;
  }

  double operator()(double score, double distance) const {
    double factor = get_value(distance);
    return score * factor;
  }

  DerivativePair operator()(double score, double deriv, double distance) const {
    double factor = get_value(distance);
    double deriv_factor = get_deriv(distance);
    return std::make_pair(score * factor,
                          score * deriv_factor + deriv * factor);
  }

  IMP_OBJECT(ForceSwitch, get_module_version_info());
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_SMOOTHING_FUNCTIONS_H */
