/**
 *  \file IMP/score_functor/OpenCubicSpline.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_RAW_OPEN_CUBIC_SPLINE_H
#define IMPSCORE_FUNCTOR_RAW_OPEN_CUBIC_SPLINE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/base_types.h>
#include <utility>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

class IMPSCOREFUNCTOREXPORT RawOpenCubicSpline {
  Floats values_, second_derivs_;
  size_t get_start_bin(double v, double, double inverse_spacing) const {
    return  static_cast<size_t>(v *inverse_spacing);
  }
  template <bool derivative>
    double compute_it(double feature, double spacing,
                      double inverse_spacing) const {
    unsigned int lowbin= std::min(get_start_bin(feature, spacing,
                                                inverse_spacing),
                                  values_.size()-2);
    size_t highbin = lowbin + 1;
    const double lowfeature = lowbin * spacing;

    const double b = (feature - lowfeature) *inverse_spacing;
    const double a = 1. - b;
    const double sixthspacing= spacing/6.0;
    double v;
    if (!derivative) {
      v= a * values_[lowbin] + b * values_[highbin]
        + (a * (a * a - 1.) * second_derivs_[lowbin]
           + b * (b * b - 1.) * second_derivs_[highbin])
      * spacing * sixthspacing;
    } else {
      v= (values_[highbin] - values_[lowbin]) *inverse_spacing
        - (3. * a * a - 1.) * sixthspacing * second_derivs_[lowbin]
        + (3. * b * b - 1.) * sixthspacing * second_derivs_[highbin];
    }
    return v;
  }
public:
  RawOpenCubicSpline(){}
  RawOpenCubicSpline(const Floats &values,
                     double spacing,
                     double inverse_spacing);

  double evaluate(double feature,
                  double spacing,
                  double inverse_spacing) const
  {
    return compute_it<false>(feature, spacing, inverse_spacing);
  }

  DerivativePair evaluate_with_derivative(double feature,
                                          double spacing,
                                          double inverse_spacing) const
  {
    return std::make_pair(compute_it<false>(feature, spacing,
                                            inverse_spacing),
                          compute_it<true>(feature, spacing,
                                           inverse_spacing));
  }
  double get_bin(double feature,
                 double spacing, double inverse_spacing) const {
    return values_[std::min(get_start_bin(feature, spacing, inverse_spacing),
                            values_.size()-1)];
  }
  double get_last() const {
    return values_.back();
  }
  double get_first() const {
    return values_.front();
  }
};

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_RAW_OPEN_CUBIC_SPLINE_H */
