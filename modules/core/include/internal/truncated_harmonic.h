/**
 *  \file truncated_harmonic.h
 *  \brief Helpers for the truncated harmonics.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H
#define IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H

#include "../config.h"

#include <IMP/base_types.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct TruncatedHarmonicData {
  TruncatedHarmonicData(Float center, Float k,
                        Float threshold, Float limit): c_(center),
  l_(limit), k_(k), t_(threshold) {
    IMP_check(.5*k*square(threshold) < limit,
              "The limit must be larger than the harmonic at the threshold",
              ValueException);
    IMP_check(k > 0, "The spring constant must be positive",
              ValueException);
    IMP_check(threshold >= 0, "The threshold must be non-negative.",
              ValueException);
    b_= .5*(-k_*k_*k_*square(t_*t_*t_)+6*square(k_)*square(t_*t_)*l_
        -12*k_*square(t_*l_)+8*l_*l_*l_)/square(k_*t_);
    o_= 2*(k_*square(t_) -l_)/(k_*t_);
    IMP_assert(std::abs(evaluate(center+t_+.001)- evaluate(center+t_-.001))
               < .1*evaluate(center+t_), "Values do not match at threshold "
               << evaluate(center+t_+.001) << " and "
               << evaluate(center+t_-.001));
    IMP_assert(o_ <t_, "Something wrong with computed values for intermediates "
               << o_ << " " << b_);
    std::cout << o_ << " " << b_ << std::endl;
  }

  inline Float evaluate(Float feature) const {
    Float x= std::abs(feature-c_);
    if (x > t_) {
      return l_ - b_/square(x-o_);
    } else {
      return .5*k_*square(x);
    }
  }

  inline FloatPair evaluate_with_derivative(Float feature) const {
    Float x= (feature-c_);
    Float deriv;
    if (std::abs(x) > t_) {
      deriv= 2*b_/(square(std::abs(x)-o_)*(std::abs(x)-o_));
      if (feature < c_) deriv=-deriv;
    } else {
      deriv= k_*x;
    }
    return FloatPair(evaluate(feature), deriv);
  }


  Float c_, l_, k_, b_, o_, t_;
};

inline std::ostream &operator<<(std::ostream &out,
                                const TruncatedHarmonicData &d) {
  out << "center: " << d.c_ << " limit: " << d.l_ << " k: " << d.k_;
  return out;
}



IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_TRUNCATED_HARMONIC_H */
