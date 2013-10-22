/**
 *  \file multifit/fitting_states.cpp
 *  \brief Fitting states
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/fitting_states.h>

IMPMULTIFIT_BEGIN_NAMESPACE

unsigned int FittingStates::get_number_of_particle_states() const {
  return states_.size();
}
void FittingStates::load_particle_state(unsigned int i, kernel::Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::RigidBody(p).set_reference_frame(states_[i]);
  p->set_value(fit_state_key_,i);
}

void FittingStates::do_show(std::ostream &out) const{
  out << fit_state_key_<<" size: " << states_.size() << std::endl;
}

IMPMULTIFIT_END_NAMESPACE
