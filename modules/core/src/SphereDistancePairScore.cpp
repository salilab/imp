/**
 *  \file SphereDistancePairScore.cpp
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
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

double NormalizedSphereDistancePairScore::evaluate_index(
    Model *m, const ParticleIndexPair &pip,
    DerivativeAccumulator *da) const {
  Float ra = m->get_attribute(radius_, std::get<0>(pip));
  Float rb = m->get_attribute(radius_, std::get<1>(pip));
  Float mr = std::min(ra, rb);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(m, std::get<0>(pip)), XYZ(m, std::get<1>(pip)), da, f_.get(),
      boost::lambda::_1 / mr - (ra + rb) / mr);
}

ModelObjectsTemp NormalizedSphereDistancePairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

WeightedSphereDistancePairScore::WeightedSphereDistancePairScore(
    UnaryFunction *f, FloatKey weight, FloatKey radius)
    : f_(f), radius_(radius), weight_(weight) {}

double WeightedSphereDistancePairScore::evaluate_index(
    Model *m, const ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  Float ra = m->get_attribute(radius_, std::get<0>(p));
  Float rb = m->get_attribute(radius_, std::get<1>(p));
  Float wa = m->get_attribute(weight_, std::get<0>(p));
  Float wb = m->get_attribute(weight_, std::get<1>(p));
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(m, std::get<0>(p)), XYZ(m, std::get<1>(p)), da, f_.get(),
      (boost::lambda::_1 - (ra + rb)) * (wa + wb));
}

ModelObjectsTemp WeightedSphereDistancePairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::SphereDistancePairScore);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::HarmonicUpperBoundSphereDistancePairScore);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::HarmonicSphereDistancePairScore);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::SoftSpherePairScore);

IMPCORE_END_NAMESPACE
