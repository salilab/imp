/**
 *  \file ExamplePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/example/ExamplePairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>

#include <boost/lambda/lambda.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

ExamplePairScore::ExamplePairScore(double x0, double k) : x0_(x0), k_(k) {}

Float ExamplePairScore::evaluate_index(Model *m,
                                       const ParticleIndexPair &pip,
                                       DerivativeAccumulator *da) const {
  // turn on logging for this method
  IMP_OBJECT_LOG;
  // assume they have coordinates
  core::XYZ d0(m, pip[0]);
  core::XYZ d1(m, pip[1]);
  // log something
  double diff =
      (d0.get_coordinates() - d1.get_coordinates()).get_magnitude() - x0_;
  IMP_LOG_VERBOSE("The distance off from x0 is " << diff << std::endl);
  double score = .5 * k_ * square(diff);
  if (da) {
    // derivatives are requested
    algebra::Vector3D delta = d0.get_coordinates() - d1.get_coordinates();
    algebra::Vector3D udelta = delta.get_unit_vector();
    double dv = k_ * diff;
    // add to the particle derivatives
    d0.add_to_derivatives(udelta * dv, *da);
    d1.add_to_derivatives(-udelta * dv, *da);
  }
  return score;
}

ModelObjectsTemp ExamplePairScore::do_get_inputs(Model *m,
                                               const ParticleIndexes &pis)
    const {
  return IMP::kernel::get_particles(m, pis);
}

IMPEXAMPLE_END_NAMESPACE
