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


Float DistancePairScore::evaluate(Particle *a, Particle *b,
                                  DerivativeAccumulator *da) const
{
  return internal::evaluate_distance_pair_score(XYZ(a),
                                                XYZ(b),
                                                da, f_.get(),
                                                boost::lambda::_1);
}

ParticlesList DistancePairScore::get_interacting_particles(Particle *a,
                                                           Particle *b) const {
  return ParticlesList(1, get_read_particles(a,b));
}

ParticlesTemp DistancePairScore::get_read_particles(Particle *a,
                                                    Particle *b) const {
  ParticlesTemp t(2);
  t[0]=a;
  t[1]=b;
  return t;
}

ParticlesTemp DistancePairScore::get_write_particles(Particle *a,
                                                     Particle *b) const {
  return get_read_particles(a,b);
}


void DistancePairScore::show(std::ostream &out) const
{
  out << "DistancePairScore using ";
  f_->show(out);
}

IMPCORE_END_NAMESPACE
