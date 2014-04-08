/**
 *  \file TransformedDistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/core/TransformedDistancePairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

TransformedDistancePairScore::TransformedDistancePairScore(
    UnaryFunction *f, const algebra::Transformation3D &t)
    : f_(f) {
  set_transformation(t);
}

struct TransformParticle {
  algebra::Vector3D tv_;
  const algebra::Rotation3D &ri_;
  XYZ d_;
  TransformParticle(const algebra::Transformation3D &t,
                    const algebra::Rotation3D &r, kernel::Model *m,
                    kernel::ParticleIndex pi)
      : ri_(r), d_(m, pi) {
    tv_ = t.get_transformed(d_.get_coordinates());
  }

  Float get_coordinate(unsigned int i) const { return tv_[i]; }

  void add_to_derivatives(const algebra::Vector3D &f,
                          DerivativeAccumulator &da) {
    IMP_LOG_VERBOSE("Incoming deriv is " << f << std::endl);
    algebra::Vector3D r = ri_.get_rotated(f);
    IMP_LOG_VERBOSE("Transformed deriv is " << r << std::endl);
    d_.add_to_derivatives(r, da);
  }
};

double TransformedDistancePairScore::evaluate_index(
    kernel::Model *m, const kernel::ParticleIndexPair &pip,
    DerivativeAccumulator *da) const {
  TransformParticle tb(t_, ri_, m, pip[1]);
  IMP_LOG_VERBOSE("Transformed particle is "
                  << tb.get_coordinate(0) << " " << tb.get_coordinate(1) << " "
                  << tb.get_coordinate(2) << std::endl);
  Float ret = internal::evaluate_distance_pair_score(
      XYZ(m, pip[0]), tb, da, f_.get(), boost::lambda::_1);
  return ret;
}

ModelObjectsTemp TransformedDistancePairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

void TransformedDistancePairScore::set_transformation(
    const algebra::Transformation3D &t) {
  ri_ = t.get_rotation().get_inverse();
  t_ = t;
}

IMPCORE_END_NAMESPACE
