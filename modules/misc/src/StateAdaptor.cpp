/**
 *  \file StateAdaptor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/misc/StateAdaptor.h"
#include "IMP/kernel/Model.h"

IMPMISC_BEGIN_NAMESPACE

StateAdaptor::StateAdaptor(kernel::Model *m, OptimizerState *before,
                           OptimizerState *after)
    : ScoreState(m, "StateAdaptor%1%"), before_(before), after_(after) {}

void StateAdaptor::do_before_evaluate() {
  if (before_) before_->update();
}

void StateAdaptor::do_after_evaluate(DerivativeAccumulator *) {
  if (after_) after_->update();
}

ModelObjectsTemp StateAdaptor::do_get_outputs() const {
  return kernel::get_particles(get_model(),
                               get_model()->get_particle_indexes());
}

ModelObjectsTemp StateAdaptor::do_get_inputs() const {
  return kernel::ModelObjectsTemp();
}

IMPMISC_END_NAMESPACE
