/**
 *  \file evaluate_distance_pair_score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_EVALUATE_DISTANCE_PAIR_SCORE_H
#define __IMP_EVALUATE_DISTANCE_PAIR_SCORE_H

#include "../Vector3D.h"
#include <boost/tuple/tuple.hpp>

IMP_BEGIN_NAMESPACE

namespace internal
{

template <class W0, class W1, class SD>
Float evaluate_distance_pair_score(W0 d0, W1 d1,
                                   DerivativeAccumulator *da,
                                   UnaryFunction *f, SD sd)
{
  static const Float MIN_DISTANCE = .00001;
  IMP_CHECK_OBJECT(f);

  Float d2 = 0;
  Vector3D delta;
  Float score;

  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
    d2 += square(delta[i]);
  }

  Float distance = std::sqrt(d2);

  Float shifted_distance = sd(distance); //scale*(distance - offset);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  if (da && distance >= MIN_DISTANCE) {
    Float deriv;

    boost::tie(score, deriv) = f->evaluate_with_derivative(shifted_distance);

    Vector3D d= delta/distance *deriv;
    d0.add_to_coordinates_derivative(d, *da);
    d1.add_to_coordinates_derivative(-d, *da);
  } else {
    // calculate the score based on the distance feature
    score = f->evaluate(shifted_distance);
  }

  return score;
}

} // namespace internal

IMP_END_NAMESPACE

#endif  /* __IMP_EVALUATE_DISTANCE_PAIR_SCORE_H */
