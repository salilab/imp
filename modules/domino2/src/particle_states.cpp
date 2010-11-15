/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/particle_states.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>

IMPDOMINO2_BEGIN_NAMESPACE
ParticleStates::~ParticleStates(){}


void ParticleStatesTable::do_show(std::ostream &out) const{}


unsigned int XYZStates::get_number_of_particle_states() const {
  return states_.size();
}
void XYZStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::XYZ(p).set_coordinates(states_[i]);
}

void XYZStates::do_show(std::ostream &out) const{}


unsigned int RigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void RigidBodyStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::RigidBody(p).set_reference_frame(states_[i]);
}

void RigidBodyStates::do_show(std::ostream &out) const{}


IMPDOMINO2_END_NAMESPACE
