/**
 *  \file DistanceToSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/DistanceToSingletonScore.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>

#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

struct StaticD
{
  algebra::Vector3D v_;
  StaticD(algebra::Vector3D v): v_(v){}
  Float get_coordinate(unsigned int i) {return v_[i];}
  void add_to_coordinates_derivative(algebra::Vector3D, DerivativeAccumulator){}
};

DistanceToSingletonScore::DistanceToSingletonScore(UnaryFunction *f,
                                                   const algebra::Vector3D &v)
    : f_(f), pt_(v){}

Float DistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  return internal::evaluate_distance_pair_score(XYZDecorator(b),
                                                StaticD(pt_), da,
                                                f_.get(), boost::lambda::_1);
}

void DistanceToSingletonScore::show(std::ostream &out) const
{
  out << "DistanceToSingletonScore using ";
  f_->show(out);
}

IMPCORE_END_NAMESPACE
