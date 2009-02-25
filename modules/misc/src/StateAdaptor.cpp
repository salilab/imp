/**
 *  \file StateAdaptor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/misc/StateAdaptor.h"


IMPMISC_BEGIN_NAMESPACE

StateAdaptor::StateAdaptor(OptimizerState *before,
                           OptimizerState *after): before_(before),
                                                   after_(after){
}

StateAdaptor::~StateAdaptor(){}

void StateAdaptor::show(std::ostream &out) const {
  out << "StateAdaptor" << std::endl;
}

void StateAdaptor::do_before_evaluate() {
  if (before_) before_->update();
}
void StateAdaptor::do_after_evaluate(DerivativeAccumulator*) {
  if (after_) after_->update();
}


IMPMISC_END_NAMESPACE
