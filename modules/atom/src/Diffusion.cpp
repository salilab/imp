/**
 *  \file Diffusion.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Diffusion.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/constants.h>
#include <IMP/atom/estimates.h>
IMPATOM_BEGIN_NAMESPACE

FloatKey Diffusion::get_diffusion_coefficient_key() {
  static FloatKey k("D");
  return k;
}

void Diffusion::do_setup_particle(Model *m,
                                  kernel::ParticleIndex pi) {
  IMP_USAGE_CHECK(core::XYZR::get_is_setup(m, pi),
                  "Particle must already be an XYZR particle");
  double r = core::XYZR(m, pi).get_radius();
  m->add_attribute(get_diffusion_coefficient_key(), pi,
                   get_einstein_diffusion_coefficient(r));
}

void Diffusion::show(std::ostream &out) const {
  XYZ::show(out);
  out << "D= " << get_diffusion_coefficient() << "A^2/fs";
}

void Diffusion::set_d(double d) {
  IMPATOM_DEPRECATED_METHOD_DEF(2.1, "Use set_diffusion_coefficient().");
  set_diffusion_coefficient(d);
}
double Diffusion::get_d() const {
  IMPATOM_DEPRECATED_METHOD_DEF(2.1, "Use set_diffusion_coefficient().");
  return get_diffusion_coefficient();
}


double get_diffusion_coefficient_from_cm2_per_second(double din) {
  unit::SquareCentimeterPerSecond dinv(din);
  unit::SquareAngstromPerFemtosecond ret = dinv;
  return ret.get_value();
}

void RigidBodyDiffusion::do_setup_particle(Model *m,
                                  kernel::ParticleIndex pi) {
  if (!Diffusion::get_is_setup(m, pi)) {
    Diffusion::setup_particle(m, pi);
  }
  core::XYZR d(m, pi);
  m->add_attribute(
                   get_rotational_diffusion_coefficient_key(), pi,
             get_einstein_rotational_diffusion_coefficient(d.get_radius()));
}
FloatKey RigidBodyDiffusion::get_rotational_diffusion_coefficient_key() {
  static FloatKey k("D rotation");
  return k;
}

void RigidBodyDiffusion::show(std::ostream &out) const {
  Diffusion::show(out);
  out << "D rotation= " << get_rotational_diffusion_coefficient() << "1/sec";
}

IMPATOM_END_NAMESPACE
