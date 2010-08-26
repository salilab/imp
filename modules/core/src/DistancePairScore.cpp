/**
 *  \file DistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>

#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

DistancePairScore::DistancePairScore(UnaryFunction *f): f_(f){}


Float DistancePairScore::evaluate(const ParticlePair &p,
                                  DerivativeAccumulator *da) const
{
  return internal::evaluate_distance_pair_score(XYZ(p[0]),
                                                XYZ(p[1]),
                                                da, f_.get(),
                                                boost::lambda::_1);
}

void DistancePairScore::do_show(std::ostream &out) const
{
  out << "function " << f_;
}



double HarmonicDistancePairScore::evaluate(const ParticlePair &p,
                            DerivativeAccumulator *da) const {
  XYZ d0(p[0]), d1(p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- x0_;
  double score= .5*k_*square(shifted_distance);
  if (!da || distance < MIN_DISTANCE) return score;
  double deriv= k_*shifted_distance;
  algebra::Vector3D uv= delta/distance;
  if (da) {
    d0.add_to_derivatives(uv*deriv, *da);
    d1.add_to_derivatives(-uv*deriv, *da);
  }

  return score;
}
void HarmonicDistancePairScore::do_show(std::ostream &out) const {
  out << "x0=" << x0_ << " and k=" << k_ << std::endl;
}

IMPCORE_END_NAMESPACE
