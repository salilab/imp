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
  static FloatKey k("atom__TAMDParticle__temperature_scale_factor");
  return k;
}

FloatKey TAMDParticle::get_friction_scale_factor_key() {
  static FloatKey k("atom__TAMDParticle__friction_scale_factor");
  return k;
}

ParticleIndexKey TAMDParticle::get_reference_particle_index_key() {
  static ParticleIndexKey k("atom__TAMDParticle__reference_particle_index");
  return k;
}

// Setup a TAMD particle with simulation temperature elevated by a factor
// tsf, and friction coefficient elevated by a factor tsf.
void TAMDParticle::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                     kernel::ParticleIndex pi_ref,
                                     float tsf,
                                     float fsf) {
  IMP_USAGE_CHECK(Diffusion::get_is_setup(m, pi) &&
                  core::XYZ::get_is_setup(m, pi),
                  "Particle must already be a Diffusion, XYZ particle");
  m->add_attribute(get_reference_particle_index_key(), pi, pi_ref);
  m->add_attribute(get_temperature_scale_factor_key(), pi, tsf);
  m->add_attribute(get_friction_scale_factor_key(), pi, fsf);
}

// copy coords from ref particle
void TAMDParticle::update_coordinates_from_ref(){
  Model* m = get_model();
  ParticleIndex pi_this = get_particle_index();
  ParticleIndex pi_ref = get_reference_particle_index();

  IMP_IF_CHECK(USAGE){
    IMP_ALWAYS_CHECK(core::XYZ::get_is_setup(m, get_particle_index()) &&
                     core::XYZ::get_is_setup(m, pi_ref),
                     "either reference particle or TAMD particle lack"
                     " XYZ coords", base::ValueException);
  }

  core::XYZ xyz_this(m, pi_this);
  core::XYZ xyz_ref(m, pi_ref);
  xyz_this.set_coordinates( xyz_ref.get_coordinates() );
}


void TAMDParticle::show(std::ostream &out) const {
  out << "temperature_scale_factor= " << get_temperature_scale_factor_key()
      << " friction_scale_factor= " << get_friction_scale_factor_key();
}

IMPATOM_END_NAMESPACE
