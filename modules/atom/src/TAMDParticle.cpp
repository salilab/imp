/**
 *  \file TAMDParticle.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/TAMDParticle.h"
#include <IMP/constants.h>

IMPATOM_BEGIN_NAMESPACE

FloatKey TAMDParticle::get_temperature_scale_factor_key() {
  static FloatKey k("TAMDParticle__temperature_scale_factor");
  return k;
}

FloatKey TAMDParticle::get_friction_scale_factor_key() {
  static FloatKey k("TAMDParticle__friction_temperature_scale_factor");
  return k;
}

// Setup a TAMD particle with simulation temperature elevated by a factor
// tsf, and friction coefficient elevated by a factor tsf.
void TAMDParticle::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                       float tsf, float fsf) {
  IMP_USAGE_CHECK(Diffusion::get_is_setup(m, pi),
                  "Particle must already be a Diffusion particle");
  m->add_attribute(get_temperature_scale_factor_key(), pi, tsf);
  m->add_attribute(get_friction_scale_factor_key(), pi, fsf);
}

void TAMDParticle::show(std::ostream &out) const {
  out << "temperature_scale_factor= " << get_temperature_scale_factor_key()
      << " friction_scale_factor= " << get_friction_scale_factor_key();
}

IMPATOM_END_NAMESPACE
