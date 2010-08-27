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


void HarmonicDistancePairScore::do_show(std::ostream &out) const {
  out << "x0=" << x0_ << " and k=" << k_ << std::endl;
}

IMPCORE_END_NAMESPACE
