/**
 *  \file evaluate_distance_pair_score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/DerivativeAccumulator.h>
#include <IMP/base_types.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>

#include <boost/tuple/tuple.hpp>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

/**
   Compute score and derivatives (if d is not null) using unary function
   f on shifted distance sd(|delta|)

   Template:
   SD - function that shifts (or transforms) distance
   D - vector dimension
   UF - a class with kernel::UnaryFunction like interface

   @param delta The delta vector from which to compute the raw distance
   @param f     Pointer to a unary function for computing the score and the
                derivatives vector
   @param d     Pointer to a D-dimensional output vector for score derivatives
                (if d is null, ignore)
   @param sd    A function that transforms/shifts d
   @param deriv_multiplier Scaling factor for derivative vector
 */
template <class SD, int D, class UF>
inline double compute_distance_pair_score(const algebra::VectorD<D> &delta,
                                          const UF *f, algebra::VectorD<D> *d,
                                          SD sd,
                                          double deriv_multiplier = 1.0) {
  static const double MIN_DISTANCE = .00001;
  double distance = delta.get_magnitude();
  double shifted_distance = sd(distance);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  DerivativePair dp;
  if (d && distance >= MIN_DISTANCE) {
    dp = f->evaluate_with_derivative(shifted_distance);
    *d = (delta / distance) * deriv_multiplier * dp.second;
  } else {
    // calculate the score based on the distance feature
    dp.first = f->evaluate(shifted_distance);
    if (d) {
      *d = dp.second * IMP::algebra::get_random_vector_on(
                           IMP::algebra::get_unit_sphere_d<3>());
    }
  }
  return dp.first;
}

template <class W0, class W1, class SD, class UF>
inline double evaluate_distance_pair_score(W0 d0, W1 d1,
                                           DerivativeAccumulator *da,
                                           const UF *f, SD sd,
                                           double deriv_multiplier = 1.0) {
  algebra::Vector3D delta;

  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }

  algebra::Vector3D d;
  double score = compute_distance_pair_score(delta, f, (da ? &d : nullptr), sd,
                                             deriv_multiplier);

  if (da) {
    d0.add_to_derivatives(d, *da);
    d1.add_to_derivatives(-d, *da);
  }

  return score;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H */
