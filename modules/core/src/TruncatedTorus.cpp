/**
 *  \file SlabWithCylindricalPore.cpp
 *  \brief Decoratr for slab particle with a cylindrical pore
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/TruncatedTorus.h>

IMPCORE_BEGIN_NAMESPACE

#define STR_MAJOR_RADIUS_KEY "torus_major_radis"
#define STR_MINOR_RADIUS_KEY "torus_minor_radis"
#define STR_THETA_KEY "truncated_torus_major_radis"

void
TruncatedTorus::do_setup_particle(IMP::Model* m,
				ParticleIndex pi,
				  double R,
				  double r,
				  double theta) {
  m->add_attribute(get_major_radius_key(), pi, R, false/*is_optimizable*/);
  m->add_attribute(get_minor_radius_key(), pi, r, false/*is_optimizable*/);
  m->add_attribute(get_theta_key(), pi, theta, false/*is_optimizable*/);
}

FloatKey TruncatedTorus::get_torus_major_radius_key() {
  static FloatKey fk(STR_MAJOR_RADIUS_KEY);
  return fk;
}

FloatKey TruncatedTorus::get_torus_minor_radius_key() {
  static FloatKey fk(STR_MINOR_RADIUS_KEY);
  return fk;
}

FloatKey TruncatedTorus::get_torus_theta_key() {
  static FloatKey fk(STR_THETA_KEY);
  return fk;
}

void TruncatedTorus::show(std::ostream &out) const {
  out <<  std::setprecision(1)
      << "TruncatedTorus R=" << get_major_radius() << " A"
      << " r=" << get_minor_radius() << " A"
      << " theta=" << get_theta() << " Radians"
      << get_thickness()
      << " ; radius=" << get_pore_radius();
}

IMPCORE_END_NAMESPACE
