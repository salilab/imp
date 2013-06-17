/**
 *  \file LennardJonesPairScore.cpp
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/LennardJonesPairScore.h>
#include <IMP/atom/smoothing_functions.h>
#include <IMP/atom/LennardJones.h>

IMPATOM_BEGIN_NAMESPACE

Float LennardJonesPairScore::evaluate_index(Model *m,
                                      const ParticleIndexPair& p,
                                      DerivativeAccumulator *da) const {
  LennardJones lj0(m, p[0]);
  LennardJones lj1(m, p[1]);

  algebra::Vector3D delta = lj0.get_coordinates() - lj1.get_coordinates();
  double distsqr = delta.get_squared_magnitude();
  double dist = std::sqrt(distsqr);
  double dist6 = distsqr * distsqr * distsqr;
  double dist12 = dist6 * dist6;

  double A, B;
  get_factors(lj0, lj1, A, B);
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

ModelObjectsTemp LennardJonesPairScore::do_get_inputs(Model *m,
                                               const ParticleIndexes &pis)
    const {
  return IMP::kernel::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
