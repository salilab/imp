/**
 *  \file RepulsiveDistancePairScore.cpp
 *  \brief A simple quadric repulsive term between two atoms. Restraint is zero
 *  when the distance equals the sum of the radii plus the shift.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#include "IMP/isd/RepulsiveDistancePairScore.h"
#include <math.h>

IMPISD_BEGIN_NAMESPACE

RepulsiveDistancePairScore::RepulsiveDistancePairScore(double d0, double k)
    : x0_(d0), k_(k) {}

double RepulsiveDistancePairScore::evaluate_index(
    Model *m, const ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  core::XYZR d0(m, p[0]), d1(m, p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  double distance2 = delta.get_squared_magnitude();
  double distance = std::sqrt(distance2);
  double target = x0_ + d0.get_radius() + d1.get_radius();
  double shifted_distance = distance - target;
  if (shifted_distance > 0) return 0;
  double energy = .5 * k_ * pow(shifted_distance, 4);
  if (da) {
    double deriv = 4 * energy / shifted_distance;
    algebra::Vector3D uv = delta / distance;
    d0.add_to_derivatives(uv * deriv, *da);
    d1.add_to_derivatives(-uv * deriv, *da);
  }
  return energy;
}

ModelObjectsTemp RepulsiveDistancePairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPISD_END_NAMESPACE
