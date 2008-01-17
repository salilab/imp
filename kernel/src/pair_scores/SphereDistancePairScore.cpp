/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/pair_scores/SphereDistancePairScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/UnaryFunction.h"

namespace IMP
{

static const Float MIN_DISTANCE = .00001;

SphereDistancePairScore::SphereDistancePairScore(UnaryFunction *f,
                                                 FloatKey radius) :
    f_(f), radius_(radius)
{
}

Float SphereDistancePairScore::evaluate(Particle *a, Particle *b,
                                        DerivativeAccumulator *da)
{
  IMP_CHECK_OBJECT(f_);

  Float d2 = 0, delta[3];
  Float score;

  XYZDecorator d0 = XYZDecorator::cast(a);
  XYZDecorator d1 = XYZDecorator::cast(b);
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
    d2 += square(delta[i]);
  }

  Float distance = std::sqrt(d2);
  Float ra = a->get_value(radius_);
  Float rb = b->get_value(radius_);
  Float shifted_distance = distance - (ra + rb);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  if (da && distance >= MIN_DISTANCE) {
    Float deriv;

    score = (*f_)(shifted_distance, deriv);

    for (int i = 0; i < 3; ++i) {
      Float d = delta[i] / distance * deriv;
      d0.add_to_coordinate_derivative(i, d, *da);
      d1.add_to_coordinate_derivative(i, -d, *da);
    }
  }

  else {
    // calculate the score based on the distance feature
    score = (*f_)(shifted_distance);
  }

  IMP_LOG(VERBOSE, "For " << a->get_index() << " and " << b->get_index()
          << " distance: " << distance << " score: " << score << std::endl);
  return score;
}

void SphereDistancePairScore::show(std::ostream &out) const
{
  out << "SphereDistancePairScore using ";
  f_->show(out);
}

} // namespace IMP
