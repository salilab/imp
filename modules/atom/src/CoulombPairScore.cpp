/**
 *  \file CoulombPairScore.cpp
 *  \brief Coulomb (electrostatic) score between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/CoulombPairScore.h>
#include <IMP/atom/smoothing_functions.h>
#include <IMP/atom/Charged.h>
#include <IMP/constants.h>

IMPATOM_BEGIN_NAMESPACE

void CoulombPairScore::calculate_multiplication_factor() {
  // 1 / (4pi * epsilon) * conversion factor to get score in kcal/mol if
  // distances are in angstroms
  static const double avogadro = 6.02214179e23;               // /mole
  static const double electron_charge = 1.6021892e-19;        // Coulomb
  static const double permittivity_vacuum = 8.854187818e-12;  // C/V/m
  static const double kcal2joule = 4186.8;

  multiplication_factor_ =
      avogadro * electron_charge * electron_charge * 1.0e10 /
      permittivity_vacuum / kcal2joule / (4.0 * PI * relative_dielectric_);
}

double CoulombPairScore::evaluate_index(Model *m,
                                        const ParticleIndexPair& p,
                                        DerivativeAccumulator *da) const {
  Charged c0(m, p[0]);
  Charged c1(m, p[1]);
  algebra::Vector3D delta = c0.get_coordinates() - c1.get_coordinates();
  double dist = delta.get_magnitude();
  double score =
      multiplication_factor_ * c0.get_charge() * c1.get_charge() / dist;
  if (da) {
    DerivativePair d = (*smoothing_function_)(score, -score / dist, dist);
    algebra::Vector3D deriv = d.second * delta / dist;
    c0.add_to_derivatives(deriv, *da);
    c1.add_to_derivatives(-deriv, *da);
    return d.first;
  } else {
    return (*smoothing_function_)(score, dist);
  }
}

ModelObjectsTemp CoulombPairScore::do_get_inputs(Model *m,
                                               const ParticleIndexes &pis)
    const {
  return IMP::kernel::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
