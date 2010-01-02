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

Float SphereDistancePairScore::evaluate(const ParticlePair &p,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in SphereDistancePairScore",
            ValueException);
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  return internal::evaluate_distance_pair_score(XYZ(p[0]),
                                                XYZ(p[1]),
                                                da, f_.get(),
                                                boost::lambda::_1-(ra+rb));
}

ParticlesList
SphereDistancePairScore
::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

ParticlesTemp SphereDistancePairScore
::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ret(2);
  ret[0]=p[0];
  ret[1]=p[1];
  return ret;
}

ContainersTemp SphereDistancePairScore
::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
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


Float NormalizedSphereDistancePairScore::evaluate(const ParticlePair &p,
                                        DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in NormalizedSphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in NormalizedSphereDistancePairScore",
            ValueException);
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float mr= std::min(ra, rb);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(XYZ(p[0]),
                                                XYZ(p[1]),
                                                da, f_.get(),
                                         boost::lambda::_1/mr-(ra+rb)/mr);
}

ParticlesList
NormalizedSphereDistancePairScore
::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

ParticlesTemp
NormalizedSphereDistancePairScore
::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ret(2);
  ret[0]=p[0];
  ret[1]=p[1];
  return ret;
}
ContainersTemp NormalizedSphereDistancePairScore
::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}

void NormalizedSphereDistancePairScore::show(std::ostream &out) const
{
  out << "NormalizedSphereDistancePairScore using ";
  f_->show(out);
}

WeightedSphereDistancePairScore::WeightedSphereDistancePairScore(
  UnaryFunction *f,FloatKey weight,FloatKey radius):
  f_(f), radius_(radius), weight_(weight){
}

Float WeightedSphereDistancePairScore::evaluate(const ParticlePair &p,
                                       DerivativeAccumulator *da) const
{
  IMP_USAGE_CHECK(p[0]->has_attribute(radius_), "Particle " << p[0]->get_name()
            << "missing radius in WeightedSphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(p[1]->has_attribute(radius_), "Particle " << p[1]->get_name()
            << "missing radius in WeightedSphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(p[0]->has_attribute(weight_), "Particle " << p[0]->get_name()
            << "missing weight in WeightedSphereDistancePairScore",
            ValueException);
  IMP_USAGE_CHECK(p[1]->has_attribute(weight_), "Particle " << p[1]->get_name()
            << "missing weight in WeightedSphereDistancePairScore",
            ValueException);
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float wa = p[0]->get_value(weight_);
  Float wb = p[1]->get_value(weight_);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
                                    XYZ(p[0]),
                                    XYZ(p[1]),
                                    da, f_.get(),
                                    (boost::lambda::_1-(ra+rb))*(wa+wb));
}

ParticlesList
WeightedSphereDistancePairScore::
  get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

ParticlesTemp
WeightedSphereDistancePairScore::
  get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ret(2);
  ret[0]=p[0];
  ret[1]=p[1];
  return ret;
}

ContainersTemp WeightedSphereDistancePairScore
::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}

void WeightedSphereDistancePairScore::show(std::ostream &out) const
{
  out << "WeightedSphereDistancePairScore using ";
  f_->show(out);
}



IMPCORE_END_NAMESPACE
