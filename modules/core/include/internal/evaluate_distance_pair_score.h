/**
 *  \file evaluate_distance_pair_score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H

#include "../config.h"
#include <IMP/DerivativeAccumulator.h>
#include <IMP/base_types.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>
#include <IMP/algebra/Vector3D.h>

#include <boost/tuple/tuple.hpp>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RawOpenCubicSpline {
  Floats values_, second_derivs_;
  size_t get_start_bin(double v, double spacing, double inverse_spacing) const {
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
  RawOpenCubicSpline(const std::vector<Float> &values,
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

template <class SD>
double compute_distance_pair_score(const algebra::VectorD<3> &delta,
                                   const UnaryFunction *f,
                                   algebra::VectorD<3> *d,
                                   SD sd,
                                   double deriv_multiplier = 1.0) {
  static const Float MIN_DISTANCE = .00001;
  double distance= delta.get_magnitude();
  double shifted_distance = sd(distance);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  DerivativePair dp;
  if (d && distance >= MIN_DISTANCE) {
    dp = f->evaluate_with_derivative(shifted_distance);
    *d= (delta/distance) * deriv_multiplier * dp.second;
  } else {
    // calculate the score based on the distance feature
    dp.first = f->evaluate(shifted_distance);
    if (d) *d= algebra::VectorD<3>(0,0,0);
  }
  return dp.first;
}


template <class W0, class W1, class SD>
double evaluate_distance_pair_score(W0 d0, W1 d1,
                                    DerivativeAccumulator *da,
                                    const UnaryFunction *f, SD sd,
                                    double deriv_multiplier = 1.0)
{
  IMP_CHECK_OBJECT(f);

  algebra::VectorD<3> delta;

  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }

  algebra::VectorD<3> d;
  double score= compute_distance_pair_score(delta, f, (da? &d : NULL), sd,
                                            deriv_multiplier);

  if (da) {
    d0.add_to_derivatives(d, *da);
    d1.add_to_derivatives(-d, *da);
  }

  return score;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H */
