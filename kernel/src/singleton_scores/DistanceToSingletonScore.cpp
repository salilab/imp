/**
 *  \file DistanceToSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/singleton_scores/DistanceToSingletonScore.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/UnaryFunction.h"
#include "IMP/internal/evaluate_distance_pair_score.h"
#include <boost/lambda/lambda.hpp>

namespace IMP
{

struct StaticD
{
  Vector3D v_;
  StaticD(Vector3D v): v_(v){}
  Float get_coordinate(unsigned int i) {return v_[i];}
  void add_to_coordinates_derivative(Vector3D, DerivativeAccumulator){}
};

DistanceToSingletonScore::DistanceToSingletonScore(UnaryFunction *f,
                                                   const Vector3D &v): f_(f),
                                                                       pt_(v){}

Float DistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  return internal::evaluate_distance_pair_score(XYZDecorator(b),
                                                StaticD(pt_), da,
                                                f_.get(), boost::lambda::_1);
}

void DistanceToSingletonScore::show(std::ostream &out) const
{
  out << "DistanceToSingletScore using ";
  f_->show(out);
}

} // namespace IMP
