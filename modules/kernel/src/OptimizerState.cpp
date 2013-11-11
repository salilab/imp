/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base//log.h"
#include "IMP/kernel/OptimizerState.h"
#include "IMP/kernel/internal/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

OptimizerState::OptimizerState(kernel::Model* m, std::string name)
    : ModelObject(m, name) {
  is_optimizing_ = false;
  reset();
  set_period(1);
}

void OptimizerState::set_optimizer(Optimizer* optimizer) {
  optimizer_ = optimizer;
}

void OptimizerState::update() {
  IMP_OBJECT_LOG;
  ++call_number_;
  IMP_LOG_TERSE("Update called with " << call_number_ << " of " << period_
                                      << " and update " << update_number_
                                      << std::endl);
  if (call_number_ % period_ == 0) {
    update_always();
  }
}
void OptimizerState::update_always() {
  IMP_OBJECT_LOG;
  do_update(update_number_);
  ++update_number_;
}
void OptimizerState::set_is_optimizing(bool tf) {
  if (!tf) {
    IMP_USAGE_CHECK(is_optimizing_, "OptimizerState " << get_name()
                                                      << " is not optimizing.");
    is_optimizing_ = false;
    if (call_number_ % period_ != 0) {
      update_always();
    }
  } else {
    IMP_USAGE_CHECK(!is_optimizing_, "OptimizerState "
                                         << get_name()
                                         << " is already optimizing.");
    is_optimizing_ = true;
    call_number_ = 0;
  }
  do_set_is_optimizing(tf);
}
void OptimizerState::set_period(unsigned int p) {
  IMP_USAGE_CHECK(p > 0, "Period must be positive.");
  period_ = p;
}

void OptimizerState::reset() {
  call_number_ = 0;
  update_number_ = 0;
}

IMPKERNEL_END_NAMESPACE
