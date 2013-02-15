/**
 *  \file DistanceToSingletonScore.cpp
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/DistanceToSingletonScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE


SphereDistanceToSingletonScore::SphereDistanceToSingletonScore(UnaryFunction *f,
                                                   const algebra::Vector3D &v)
    : f_(f), pt_(v){}

Float SphereDistanceToSingletonScore::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  Float v= internal::evaluate_distance_pair_score(XYZR(b),
                                                  StaticD(pt_), da,
                                                  f_.get(),
                                                  boost::lambda::_1
                                                  - XYZR(b).get_radius());
  IMP_LOG_VERBOSE( "SphereDistanceTo from " << XYZR(b) << " to "
          << pt_ << " scored " << v << std::endl);
  return v;
}

void SphereDistanceToSingletonScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}

IMPCORE_END_NAMESPACE
