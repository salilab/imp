/**
 *  \file StateAdaptor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/misc/StateAdaptor.h"


IMPMISC_BEGIN_NAMESPACE

StateAdaptor::StateAdaptor(OptimizerState *before,
                           OptimizerState *after): before_(before),
                                                   after_(after){
}

void StateAdaptor::do_show(std::ostream &out) const {
}

void StateAdaptor::do_before_evaluate() {
  if (before_) before_->update();
}
void StateAdaptor::do_after_evaluate(DerivativeAccumulator*) {
  if (after_) after_->update();
}


ParticlesList StateAdaptor::get_interacting_particles() const {
  return ParticlesList();
}

ParticlesTemp StateAdaptor::get_input_particles() const {
  return ParticlesTemp(get_model()->particles_begin(),
                       get_model()->particles_end());
}


ParticlesTemp StateAdaptor::get_output_particles() const {
  return ParticlesTemp();
}

ContainersTemp StateAdaptor::get_input_containers() const {
  return ContainersTemp();
}


ContainersTemp StateAdaptor::get_output_containers() const {
  return ContainersTemp();
}

IMPMISC_END_NAMESPACE
