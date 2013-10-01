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
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  return ret;
}

NormalizedSphereDistancePairScore::NormalizedSphereDistancePairScore(
    UnaryFunction *f, FloatKey radius)
    : f_(f), radius_(radius) {}

double NormalizedSphereDistancePairScore::evaluate_index(
    kernel::Model *m, const kernel::ParticleIndexPair &pip,
    DerivativeAccumulator *da) const {
  Float ra = m->get_attribute(radius_, pip[0]);
  Float rb = m->get_attribute(radius_, pip[1]);
  Float mr = std::min(ra, rb);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(m, pip[0]), XYZ(m, pip[1]), da, f_.get(),
      boost::lambda::_1 / mr - (ra + rb) / mr);
}

ModelObjectsTemp NormalizedSphereDistancePairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

WeightedSphereDistancePairScore::WeightedSphereDistancePairScore(
    UnaryFunction *f, FloatKey weight, FloatKey radius)
    : f_(f), radius_(radius), weight_(weight) {}

double WeightedSphereDistancePairScore::evaluate_index(
    kernel::Model *m, const kernel::ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  Float ra = m->get_attribute(radius_, p[0]);
  Float rb = m->get_attribute(radius_, p[1]);
  Float wa = m->get_attribute(weight_, p[0]);
  Float wb = m->get_attribute(weight_, p[1]);
  // lambda is inefficient due to laziness
  return internal::evaluate_distance_pair_score(
      XYZ(m, p[0]), XYZ(m, p[1]), da, f_.get(),
      (boost::lambda::_1 - (ra + rb)) * (wa + wb));
}

ModelObjectsTemp WeightedSphereDistancePairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
