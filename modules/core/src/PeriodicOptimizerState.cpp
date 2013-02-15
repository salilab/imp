/**
 *  \file LeavesRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/PeriodicOptimizerState.h>

IMPCORE_BEGIN_NAMESPACE
PeriodicOptimizerState::PeriodicOptimizerState(std::string name):
  OptimizerState(name){
  reset();
  set_period(1);
}
 void PeriodicOptimizerState::update() {
    IMP_OBJECT_LOG;
    ++call_number_;
    IMP_LOG_TERSE( "Update called with " << call_number_
            << " of " << period_ << " and update "
            << update_number_ << std::endl);
    if (call_number_%(period_) ==0) {
      update_always();
    }
  }
void PeriodicOptimizerState::update_always() {
  IMP_OBJECT_LOG;
  do_update(update_number_);
  ++update_number_;
}
  void PeriodicOptimizerState::set_is_optimizing(bool tf) {
    if (!tf) {
      do_update(update_number_);
      ++update_number_;
    } else {
      call_number_=0;
    }
  }
  void PeriodicOptimizerState::set_period(unsigned int p) {
    IMP_USAGE_CHECK(p>0, "Period must be positive.");
    period_=p; call_number_=0;
  }
  unsigned int PeriodicOptimizerState::get_period() const {
    return period_;
  }
  void PeriodicOptimizerState::reset() {
    call_number_=0;
    update_number_=0;
  }
IMPCORE_END_NAMESPACE
