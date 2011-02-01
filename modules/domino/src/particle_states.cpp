/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>

IMPDOMINO_BEGIN_NAMESPACE
ParticleStates::~ParticleStates(){}


void ParticleStatesTable::do_show(std::ostream &out) const{
  for (Map::const_iterator it= enumerators_.begin(); it != enumerators_.end();
       ++it) {
    out << it->first->get_name() << ": " << it->second->get_name()
        << std::endl;
  }
}


unsigned int TrivialStates::get_number_of_particle_states() const {
  return n_;
}
void TrivialStates::load_particle_state(unsigned int i, Particle *p) const {
  p->set_value(k_, i);
}

void TrivialStates::do_show(std::ostream &out) const{
  out << "size: " << n_ << std::endl;
}



unsigned int XYZStates::get_number_of_particle_states() const {
  return states_.size();
}
void XYZStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::XYZ(p).set_coordinates(states_[i]);
}

void XYZStates::do_show(std::ostream &out) const{
  out << "size: " << states_.size() << std::endl;
}


unsigned int RigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void RigidBodyStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::RigidBody(p).set_reference_frame(states_[i]);
}

void RigidBodyStates::do_show(std::ostream &out) const{
  out << "size: " << states_.size() << std::endl;
}


IMPDOMINO_END_NAMESPACE
