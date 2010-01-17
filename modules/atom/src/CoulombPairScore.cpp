/**
 *  \file CoulombPairScore.cpp
 *  \brief Coulomb (electrostatic) score between a pair of particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/atom/CoulombPairScore.h>
#include <IMP/atom/Charged.h>
#include <IMP/constants.h>

IMPATOM_BEGIN_NAMESPACE

void CoulombPairScore::calculate_multiplication_factor()
{
  // 1 / (4pi * epsilon) * conversion factor to get score in kcal/mol if
  // distances are in angstroms
  static const double avogadro = 6.02214179e23; // /mole
  static const double electron_charge = 1.6021892e-19; // Coulomb
  static const double permittivity_vacuum = 8.854187818e-12; // C/V/m
  static const double kcal2joule = 4186.8;

  multiplication_factor_ = avogadro * electron_charge * electron_charge
                           * 1.0e10 / permittivity_vacuum / kcal2joule
                           / (4.0 * PI * relative_dielectric_);
}

Float CoulombPairScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  Charged c0(p[0]);
  Charged c1(p[1]);
  algebra::Vector3D delta = c0.get_coordinates() - c1.get_coordinates();
  double sqrmag = delta.get_squared_magnitude();
  double factor = multiplication_factor_ * c0.get_charge() * c1.get_charge()
                  / std::sqrt(sqrmag);
  if (da) {
    algebra::Vector3D d = - factor * delta / sqrmag;
    c0.add_to_derivatives(d, *da);
    c1.add_to_derivatives(-d, *da);
  }
  // todo: switch/shift function
  return factor;
}

ParticlesList CoulombPairScore
::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

ParticlesTemp CoulombPairScore
::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp t(2);
  t[0]=p[0];
  t[1]=p[1];
  return t;
}

ContainersTemp CoulombPairScore
::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}

void CoulombPairScore::show(std::ostream &out) const
{
  out << "CoulombPairScore with relative dielectric " << relative_dielectric_;
}

IMPATOM_END_NAMESPACE
