/**
 *  \file log.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_SET_LOG_STATE_H
#define IMP_SET_LOG_STATE_H

#include "log.h"
#include "Object.h"

IMP_BEGIN_NAMESPACE


//! A class to change and restore log state
/**
   To use, create an instance of this class which the log level you
   want. When it goes out of scope, it will restore the old level.
   \ingroup log
 */
class IMPEXPORT SetLogState: public RAII
{
  LogLevel level_;
  LogTarget target_;
  Object* obj_;
public:
  //! Construct it with the desired level and target
  SetLogState(LogLevel l, LogTarget t= get_log_target()):
    target_(get_log_target()), obj_(NULL) {
    if (l != DEFAULT) {
      level_= get_log_level();
      internal::Log::get().set_level(l);
      set_log_target(t);
    } else {
      level_=DEFAULT;
    }
  }
  //! Control the log level of the object instead of the global one
  SetLogState(Object *o, LogLevel l):
    target_(get_log_target()), obj_(o) {
    if (l != DEFAULT) {
      level_= obj_->get_log_level();
      obj_->set_log_level(l);
    } else {
      level_=DEFAULT;
    }
  }
  ~SetLogState() {
    if (level_ != DEFAULT) {
      if (obj_) {
        obj_->set_log_level(level_);
      } else {
        internal::Log::get().set_level(level_);
        set_log_target(target_);
      }
    }
  }
};


IMP_END_NAMESPACE

#endif  /* IMP_SET_LOG_STATE_H */
