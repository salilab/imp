/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/SetLogState.h>
#include <IMP/Object.h>
#include <IMP/log.h>

IMPKERNEL_BEGIN_NAMESPACE
void SetLogState::do_set(Object *o, LogLevel l) {
  if (l != DEFAULT) {
    obj_ = o;
    level_ = obj_->get_log_level();
    obj_->set_log_level(l);
  } else {
    obj_ = nullptr;
    level_ = DEFAULT;
  }
}

SetLogState::SetLogState(LogLevel l) {
  obj_ = nullptr;
  level_ = DEFAULT;
  set(l);
}
void SetLogState::set(LogLevel l) {
  reset();
  if (l != DEFAULT) {
    level_ = get_log_level();
    set_log_level(l);
  } else {
    level_ = DEFAULT;
  }
}

void SetLogState::do_reset() {
  if (level_ != DEFAULT) {
    if (obj_) {
      obj_->set_log_level(level_);
    } else {
      set_log_level(level_);
    }
    obj_ = nullptr;
    level_ = DEFAULT;
  }
}

void SetLogState::do_show(std::ostream &out) const {
  out << "Setting from " << level_ << " to "
      << (obj_ ? obj_->get_log_level() : IMP::get_log_level())
      << std::endl;
}

IMPKERNEL_END_NAMESPACE
