/**
 *  \file StateAdaptor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/misc/StateAdaptor.h"
#include "IMP/Model.h"

IMPMISC_BEGIN_NAMESPACE

StateAdaptor::StateAdaptor(Model *m, OptimizerState *before,
                           OptimizerState *after)
    : ScoreState(m, "StateAdaptor%1%"), before_(before), after_(after) {}

void StateAdaptor::do_before_evaluate() {
  if (before_) before_->update();
}

void StateAdaptor::do_after_evaluate(DerivativeAccumulator *) {
  if (after_) after_->update();
}

ModelObjectsTemp StateAdaptor::do_get_outputs() const {
  return ModelObjectsTemp(get_model()->particles_begin(),
                          get_model()->particles_end());
}

ModelObjectsTemp StateAdaptor::do_get_inputs() const {
  return ModelObjectsTemp();
}

IMPMISC_END_NAMESPACE
