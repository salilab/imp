/**
 *  \file DistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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

ParticlesList DistancePairScore
::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

ParticlesTemp DistancePairScore
::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp t(2);
  t[0]=p[0];
  t[1]=p[1];
  return t;
}

ContainersTemp DistancePairScore
::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}


void DistancePairScore::show(std::ostream &out) const
{
  out << "DistancePairScore using ";
  f_->show(out);
}

IMPCORE_END_NAMESPACE
