/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/particle_states.h>
#include <IMP/core/XYZ.h>

IMPDOMINO2_BEGIN_NAMESPACE
ParticleStates::~ParticleStates(){}


void ParticleStatesTable::do_show(std::ostream &out) const{}


unsigned int XYZsStates::get_number_of_states() const {
  return states_.size();
}
void XYZsStates::load_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::XYZ(p).set_coordinates(states_[i]);
}

void XYZsStates::do_show(std::ostream &out) const{}

IMPDOMINO2_END_NAMESPACE
