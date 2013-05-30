/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

HarmonicUpperBoundSphereDiameterPairScore::
    HarmonicUpperBoundSphereDiameterPairScore(double d0, double k)
    : x0_(d0), k_(k) {}

ModelObjectsTemp HarmonicUpperBoundSphereDiameterPairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  return ret;
}

NormalizedSphereDistancePairScore::NormalizedSphereDistancePairScore(
    UnaryFunction *f, FloatKey radius)
    : f_(f), radius_(radius) {}

Float NormalizedSphereDistancePairScore::evaluate(
    const ParticlePair &p, DerivativeAccumulator *da) const {
  IMP_USAGE_CHECK(
      p[0]->has_attribute(radius_),
      "Particle " << p[0]->get_name()
                  << "missing radius in NormalizedSphereDistancePairScore");
  IMP_USAGE_CHECK(
      p[1]->has_attribute(radius_),
      "Particle " << p[1]->get_name()
                  << "missing radius in NormalizedSphereDistancePairScore");
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float mr = std::min(ra, rb);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(p[0]), XYZ(p[1]), da, f_.get(),
      boost::lambda::_1 / mr - (ra + rb) / mr);
}

void NormalizedSphereDistancePairScore::do_show(std::ostream &out) const {
  out << "function " << *f_ << std::endl;
}

WeightedSphereDistancePairScore::WeightedSphereDistancePairScore(
    UnaryFunction *f, FloatKey weight, FloatKey radius)
    : f_(f), radius_(radius), weight_(weight) {}

Float WeightedSphereDistancePairScore::evaluate(
    const ParticlePair &p, DerivativeAccumulator *da) const {
  IMP_USAGE_CHECK(
      p[0]->has_attribute(radius_),
      "Particle " << p[0]->get_name()
                  << "missing radius in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(
      p[1]->has_attribute(radius_),
      "Particle " << p[1]->get_name()
                  << "missing radius in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(
      p[0]->has_attribute(weight_),
      "Particle " << p[0]->get_name()
                  << "missing weight in WeightedSphereDistancePairScore");
  IMP_USAGE_CHECK(
      p[1]->has_attribute(weight_),
      "Particle " << p[1]->get_name()
                  << "missing weight in WeightedSphereDistancePairScore");
  Float ra = p[0]->get_value(radius_);
  Float rb = p[1]->get_value(radius_);
  Float wa = p[0]->get_value(weight_);
  Float wb = p[1]->get_value(weight_);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(p[0]), XYZ(p[1]), da, f_.get(),
      (boost::lambda::_1 - (ra + rb)) * (wa + wb));
}

void WeightedSphereDistancePairScore::do_show(std::ostream &out) const {
  out << "function " << *f_ << std::endl;
}

IMPCORE_END_NAMESPACE
