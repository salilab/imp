/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

SphereDistancePairScore::SphereDistancePairScore(UnaryFunction *f,
                                                 FloatKey radius) :
    f_(f), radius_(radius)
{
}
namespace {
struct Shift
{
  Float s_;
  Shift(Float s): s_(s){}
  Float operator()(Float t) const {return t-s_;}
};
}

Float SphereDistancePairScore::evaluate(Particle *a, Particle *b,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(a->has_attribute(radius_), "Particle " << a->get_name()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(b->has_attribute(radius_), "Particle " << b->get_name()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  Float ra = a->get_value(radius_);
  Float rb = b->get_value(radius_);
  return internal::evaluate_distance_pair_score(XYZ(a),
                                                XYZ(b),
                                                da, f_.get(),
                                                boost::lambda::_1-(ra+rb));
}

ParticlesList
SphereDistancePairScore::get_interacting_particles(Particle *a,
                                                   Particle *b) const {
  return ParticlesList(1, get_used_particles(a,b));
}

ParticlesTemp SphereDistancePairScore::get_used_particles(Particle *a,
                                                          Particle *b) const {
  ParticlesTemp ret(2);
  ret[0]=a;
  ret[1]=b;
  return ret;
}


void SphereDistancePairScore::show(std::ostream &out) const
{
  out << "SphereDistancePairScore using ";
  f_->show(out);
}







NormalizedSphereDistancePairScore
::NormalizedSphereDistancePairScore(UnaryFunction *f,
                                    FloatKey radius) :
    f_(f), radius_(radius)
{
}


Float NormalizedSphereDistancePairScore::evaluate(Particle *a, Particle *b,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(a->has_attribute(radius_), "Particle " << a->get_name()
            << "missing radius in NormalizedSphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(b->has_attribute(radius_), "Particle " << b->get_name()
            << "missing radius in NormalizedSphereDistancePairScore",
            ValueException);
  Float ra = a->get_value(radius_);
  Float rb = b->get_value(radius_);
  Float mr= std::min(ra, rb);
  return internal::evaluate_distance_pair_score(XYZ(a),
                                                XYZ(b),
                                                da, f_.get(),
                                         boost::lambda::_1/mr-(ra+rb)/mr);
}

ParticlesList
NormalizedSphereDistancePairScore::get_interacting_particles(Particle *a,
                                                       Particle *b) const {
  return ParticlesList(1, get_used_particles(a,b));
}

ParticlesTemp
NormalizedSphereDistancePairScore::get_used_particles(Particle *a,
                                                      Particle *b) const {
  ParticlesTemp ret(2);
  ret[0]=a;
  ret[1]=b;
  return ret;
}


void NormalizedSphereDistancePairScore::show(std::ostream &out) const
{
  out << "NormalizedSphereDistancePairScore using ";
  f_->show(out);
}

IMPCORE_END_NAMESPACE
