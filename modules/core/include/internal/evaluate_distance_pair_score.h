/**
 *  \file evaluate_distance_pair_score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H

#include "../macros.h"

#include <IMP/utility.h>
#include <IMP/algebra/Vector3D.h>

#include <boost/tuple/tuple.hpp>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

template <class SD>
Float compute_distance_pair_score(const algebra::Vector3D &delta,
                                  const UnaryFunction *f,
                                  algebra::Vector3D *d,
                                  SD sd) {
  static const Float MIN_DISTANCE = .00001;
  Float distance= delta.get_magnitude();
  Float shifted_distance = sd(distance);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  Float score, deriv;
  if (d && distance >= MIN_DISTANCE) {
    boost::tie(score, deriv) = f->evaluate_with_derivative(shifted_distance);
    *d= (delta/distance) *deriv;
  } else {
    // calculate the score based on the distance feature
    score = f->evaluate(shifted_distance);
    if (d) *d= algebra::Vector3D(0,0,0);
  }
  return score;
}


template <class W0, class W1, class SD>
Float evaluate_distance_pair_score(W0 d0, W1 d1,
                                   DerivativeAccumulator *da,
                                   const UnaryFunction *f, SD sd)
{
  IMP_CHECK_OBJECT(f);

  algebra::Vector3D delta;

  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }

#ifndef NDEBUG
  algebra::Vector3D d(std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::quiet_NaN());
#else
  algebra::Vector3D d;
#endif
  Float score= compute_distance_pair_score(delta, f, (da? &d : NULL), sd);


  if (da) {
    d0.add_to_derivatives(d, *da);
    d1.add_to_derivatives(-d, *da);
  }

  return score;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_EVALUATE_DISTANCE_PAIR_SCORE_H */
