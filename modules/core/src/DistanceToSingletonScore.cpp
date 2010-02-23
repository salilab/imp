/**
 *  \file DistanceToSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/core/DistanceToSingletonScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>

#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

struct StaticD
{
  algebra::VectorD<3> v_;
  StaticD(algebra::VectorD<3> v): v_(v){}
  Float get_coordinate(unsigned int i) {return v_[i];}
  void add_to_derivatives(algebra::VectorD<3> v, DerivativeAccumulator){
    IMP_LOG(VERBOSE, "DistanceTo dropped deriv of " <<  v << std::endl);
  }
};

DistanceToSingletonScore::DistanceToSingletonScore(UnaryFunction *f,
                                                   const algebra::VectorD<3> &v)
    : f_(f), pt_(v){}

Float DistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  Float v= internal::evaluate_distance_pair_score(XYZ(b),
                                                  StaticD(pt_), da,
                                                  f_.get(), boost::lambda::_1);
  IMP_LOG(VERBOSE, "DistanceTo from " << XYZ(b) << " to "
          << pt_ << " scored " << v << std::endl);
  return v;
}

void DistanceToSingletonScore::do_show(std::ostream &out) const
{
  out << "function " << *f_;
}


SphereDistanceToSingletonScore::SphereDistanceToSingletonScore(UnaryFunction *f,
                                                   const algebra::VectorD<3> &v)
    : f_(f), pt_(v){}

Float SphereDistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  Float v= internal::evaluate_distance_pair_score(XYZR(b),
                                                  StaticD(pt_), da,
                                                  f_.get(),
                                                  boost::lambda::_1
                                                  - XYZR(b).get_radius());
  IMP_LOG(VERBOSE, "SphereDistanceTo from " << XYZR(b) << " to "
          << pt_ << " scored " << v << std::endl);
  return v;
}

void SphereDistanceToSingletonScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}

IMPCORE_END_NAMESPACE
