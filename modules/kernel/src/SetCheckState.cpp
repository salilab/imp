/**
 *  \file Check.cpp   \brief Checking and error reporting support.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/SetCheckState.h>
#include <IMP/Object.h>
#include <IMP/exception.h>

IMPKERNEL_BEGIN_NAMESPACE
void SetCheckState::do_set(Object *o, CheckLevel l) {
  if (l != DEFAULT_CHECK) {
    obj_ = o;
    level_ = obj_->get_check_level();
    obj_->set_check_level(l);
  } else {
    obj_ = nullptr;
    level_ = DEFAULT_CHECK;
  }
}

void SetCheckState::do_reset() {
  if (level_ != DEFAULT_CHECK) {
    if (obj_) {
      obj_->set_check_level(level_);
    } else {
      set_check_level(level_);
    }
    obj_ = nullptr;
    level_ = DEFAULT_CHECK;
  }
}

void SetCheckState::do_show(std::ostream &out) const {
  out << "Setting from " << level_ << " to "
      << (obj_ ? obj_->get_check_level() : IMP::get_check_level())
      << std::endl;
}
void SetCheckState::set(CheckLevel l) {
  reset();
  if (l != DEFAULT_CHECK) {
    level_ = get_check_level();
    set_check_level(l);
  } else {
    level_ = DEFAULT_CHECK;
  }
}

IMPKERNEL_END_NAMESPACE
