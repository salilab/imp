/**
 *  \file IMP/atom/smoothing_functions.h
 *  \brief Classes to smooth nonbonded interactions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SMOOTHING_FUNCTIONS_H
#define IMPATOM_SMOOTHING_FUNCTIONS_H

#include <IMP/atom/atom_config.h>

#include <IMP/base_types.h>
#include <IMP/base/Object.h>

IMPATOM_BEGIN_NAMESPACE

//! Base class for smoothing nonbonded interactions as a function of distance.
/** The class is given the score (and optionally its first derivative)
    at a given distance and returns a smoothed form of the score.
    Smoothing functions are used to avoid a discontinuity in the scoring
    function and/or its derivatives at the cutoff distance (the distance
    threshold used by IMP::core::ClosePairsFinder), as this can lead
    to nonphysical motions of the system. They are used by physical scoring
    functions that drop off slowly with distance, such as CoulombPairScore,
    in combination with a ClosePairsFinder.

    Smoothing functions usually offset the score by a constant value
    (a shift function) or smooth it from its normal value to zero over
    a defined range (a switch function, such as ForceSwitch).
 */
class IMPATOMEXPORT SmoothingFunction : public IMP::base::Object {
 public:
  SmoothingFunction();

  //! Smooth the score at a given distance.
  /** \return the smoothed score.
   */
  virtual double operator()(double score, double distance) const = 0;

  //! Smooth the score and its first derivative at a given distance.
  /** \return a DerivativePair containing the smoothed score and its
              first derivative.
   */
  virtual DerivativePair operator()(double score, double deriv,
                                    double distance) const = 0;

  IMP_REF_COUNTED_DESTRUCTOR(SmoothingFunction);
};

//! Smooth interaction scores by switching the derivatives (force switch).
/** This function leaves the scores unaffected for distances below or equal
    to min_distance, returns zero for distances above max_distance, and between
    the two thresholds smoothes the score such that its first derivatives drop
    linearly, i.e. the score is simply multiplied by \f[
       \left\{
       \begin{array}{ll}
             1 & d \leq d_{min} \\
             \frac{(d_{max} - d)^2 (d_{max} + 2d - 3d_{min})}
                  {(d_{max} - d_{min})^3} & d_{min} < d \leq d_{max} \\
             0 & d > d_{max} \\
       \end{array}
       \right.
    \f] where \f$d\f$ is the distance, and \f$d_{min}\f$ and \f$d_{max}\f$ are
    the thresholds set in the ForceSwitch constructor.

    This behavior is roughly equivalent to CHARMM's force switch nonbonded
    interaction smoothing (which is also the smoothing mechanism used
    by MODELLER).

    \see CoulombPairScore
 */
class IMPATOMEXPORT ForceSwitch : public SmoothingFunction {
  double min_distance_, max_distance_;
  double value_prefactor_, deriv_prefactor_;

  inline double get_value(double distance) const {
    if (distance <= min_distance_) {
      return 1.0;
    } else if (distance > max_distance_) {
      return 0.0;
    } else {
      double d = max_distance_ - distance;
      return value_prefactor_ * d * d *
             (max_distance_ + 2.0 * distance - 3.0 * min_distance_);
    }
  }

  inline double get_deriv(double distance) const {
    if (distance <= min_distance_ || distance > max_distance_) {
      return 0.0;
    } else {
      return deriv_prefactor_ * (max_distance_ - distance) *
             (min_distance_ - distance);
    }
  }

 public:
  ForceSwitch(double min_distance, double max_distance)
      : min_distance_(min_distance), max_distance_(max_distance) {
    IMP_USAGE_CHECK(max_distance > min_distance,
                    "max_distance should be greater than min_distance");
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

  IMP_OBJECT(ForceSwitch);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SMOOTHING_FUNCTIONS_H */
