/**
 *  \file truncated_harmonic.h
 *  \brief Helpers for the truncated harmonics.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H
#define IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H

#include <IMP/core/core_config.h>

#include <IMP/base_types.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct TruncatedHarmonicData {
  TruncatedHarmonicData(double center, double k, double threshold, double limit)
      : c_(center), l_(limit), k_(k), t_(threshold) {
    IMP_USAGE_CHECK(
        .5 * k * square(threshold) < limit,
        "The limit (" << limit
        << ") must be larger than the value of the harmonic ("
        << .5 * k * square(threshold) << ") at the threshold ("
        << threshold << ")");
    IMP_USAGE_CHECK(k > 0, "The spring constant must be positive");
    IMP_USAGE_CHECK(threshold >= 0, "The threshold must be non-negative.");
    b_ = .5 * (-k_ * k_ * k_ * square(t_ * t_ * t_) +
               6 * square(k_) * square(t_ * t_) * l_ -
               12 * k_ * square(t_ * l_) + 8 * l_ * l_ * l_) /
         square(k_ * t_);
    o_ = 2 * (k_ * square(t_) - l_) / (k_ * t_);
    IMP_INTERNAL_CHECK(
        std::abs(evaluate(center + t_ + .001) - evaluate(center + t_ - .001)) <
            .1 * evaluate(center + t_) + .01,
        "Values do not match at threshold " << evaluate(center + t_ + .001)
                                            << " and "
                                            << evaluate(center + t_ - .001));
    IMP_INTERNAL_CHECK(o_ < t_,
                       "Something wrong with computed values for intermediates "
                           << o_ << " " << b_);
    // std::cout << o_ << " " << b_ << std::endl;
  }

  inline double evaluate(double feature) const {
    double x = std::abs(feature - c_);
    if (x > t_) {
      return l_ - b_ / square(x - o_);
    } else {
      return .5 * k_ * square(x);
    }
  }

  inline DerivativePair evaluate_with_derivative(double feature) const {
    double x = (feature - c_);
    double deriv;
    if (std::abs(x) > t_) {
      deriv = 2 * b_ / (square(std::abs(x) - o_) * (std::abs(x) - o_));
      if (feature < c_) deriv = -deriv;
    } else {
      deriv = k_ * x;
    }
    return DerivativePair(evaluate(feature), deriv);
  }

  double c_, l_, k_, b_, o_, t_;
};

inline std::ostream &operator<<(std::ostream &out,
                                const TruncatedHarmonicData &d) {
  out << "center: " << d.c_ << " limit: " << d.l_ << " k: " << d.k_;
  return out;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H */
