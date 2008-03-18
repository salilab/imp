/**
 *  \file DistanceToSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/singleton_scores/DistanceToSingletonScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/UnaryFunction.h"

namespace IMP
{

namespace internal
{

static const Float MIN_DISTANCE = .00001;
Float evaluate_distance_to_singleton_score(const Vector3D &v, 
                                           Particle *b,
                                           DerivativeAccumulator *da,
                                           UnaryFunction *f,
                                           Float offset,
                                           Float scale)
{
  IMP_CHECK_OBJECT(f);
  IMP_CHECK_OBJECT(b);

  Float d2 = 0, delta[3];
  Float score;

  XYZDecorator d1 = XYZDecorator::cast(b);
  if (!d1.get_coordinates_are_optimized()) {
    IMP_WARN("DistanceToSingletonScore called on non optimized particle"
             << b->get_index() <<std::endl);
    return 0;
  }
  for (int i = 0; i < 3; ++i) {
    delta[i] = v[i] - d1.get_coordinate(i);
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
      d1.add_to_coordinate_derivative(i, -d, *da);
    }
  } else {
    // calculate the score based on the distance feature
    score = f->evaluate(shifted_distance);
  }

  return score;
}

} // namespace internal

DistanceToSingletonScore::DistanceToSingletonScore(UnaryFunction *f,
                                                   const Vector3D &v): f_(f),
                                                                       pt_(v){}

Float DistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da)
{
  return internal::evaluate_distance_to_singleton_score(pt_,b, da,
                                                        f_.get(), 0,1);
}

void DistanceToSingletonScore::show(std::ostream &out) const
{
  out << "DistanceToSingletScore using ";
  f_->show(out);
}

} // namespace IMP
