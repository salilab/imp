/**
 *  \file LennardJonesTypedPairScore.cpp
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/LennardJonesTypedPairScore.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

Float LennardJonesTypedPairScore::evaluate_index(
      Model *m, const ParticleIndexPair &p, DerivativeAccumulator *da) const {
  LennardJonesTyped lj0(m, std::get<0>(p));
  LennardJonesTyped lj1(m, std::get<1>(p));

  algebra::Vector3D delta = lj0.get_coordinates() - lj1.get_coordinates();
  double distsqr = delta.get_squared_magnitude();
  double dist = std::sqrt(distsqr);
  double dist6 = distsqr * distsqr * distsqr;
  double dist12 = dist6 * dist6;

  int index = params_->get_parameter_index(lj0.get_index(), lj1.get_index());
  double A = params_->aij_[index] * repulsive_weight_;
  double B = params_->bij_[index] * attractive_weight_;
  double repulsive = A / dist12;
  double attractive = B / dist6;
  double score = repulsive - attractive;

  if (da) {
    DerivativePair d = (*smoothing_function_)(
        score, (6.0 * attractive - 12.0 * repulsive) / dist, dist);
    algebra::Vector3D deriv = d.second * delta / dist;
    lj0.add_to_derivatives(deriv, *da);
    lj1.add_to_derivatives(-deriv, *da);
    return d.first;
  } else {
    return (*smoothing_function_)(score, dist);
  }
}

ModelObjectsTemp LennardJonesTypedPairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
