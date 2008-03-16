/**
 *  \file DistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/DistancePairScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/UnaryFunction.h"

namespace IMP
{

namespace internal
{

static const Float MIN_DISTANCE = .00001;
Float evaluate_distance_pair_score(Particle *a, Particle *b,
                                   DerivativeAccumulator *da,
                                   UnaryFunction *f,
                                   Float offset,
                                   Float scale)
{
  IMP_CHECK_OBJECT(f);
  IMP_CHECK_OBJECT(a);
  IMP_CHECK_OBJECT(b);

  Float d2 = 0, delta[3];
  Float score;

  XYZDecorator d0 = XYZDecorator::cast(a);
  XYZDecorator d1 = XYZDecorator::cast(b);
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
    d2 += square(delta[i]);
  }

  Float distance = std::sqrt(d2);

  Float shifted_distance = scale*(distance - offset);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  if (da && distance >= MIN_DISTANCE) {
    Float deriv;

    score = f->evaluate_deriv(shifted_distance, deriv);

    for (int i = 0; i < 3; ++i) {
      Float d = delta[i] / distance * deriv;
      d0.add_to_coordinate_derivative(i, d, *da);
      d1.add_to_coordinate_derivative(i, -d, *da);
    }
  } else {
    // calculate the score based on the distance feature
    score = f->evaluate(shifted_distance);
  }

  return score;
}

} // namespace internal

DistancePairScore::DistancePairScore(UnaryFunction *f): f_(f){}

Float DistancePairScore::evaluate(Particle *a, Particle *b,
                                  DerivativeAccumulator *da)
{
  return internal::evaluate_distance_pair_score(a,b, da, f_.get(), 0,1);
}

void DistancePairScore::show(std::ostream &out) const
{
  out << "DistancePairScore using ";
  f_->show(out);
}

} // namespace IMP
