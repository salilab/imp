/**
 *  \file SetLogState.h     \brief Logging and error reporting support.
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
   \ingroup logging
 */
class IMPEXPORT SetLogState: public RAII
{
  LogLevel level_;
  Object* obj_;
public:
  IMP_RAII(SetLogState, (Object *o, LogLevel l),
           {level_= DEFAULT; obj_=NULL;},
           {
             if (l != DEFAULT) {
               obj_=o;
               level_= obj_->get_log_level();
               obj_->set_log_level(l);
             } else {
               obj_=NULL;
               level_=DEFAULT;
             }
           },
           {
             if (level_ != DEFAULT) {
               if (obj_) {
                 obj_->set_log_level(level_);
               } else {
                 set_log_level(level_);
               }
               obj_=NULL;
               level_=DEFAULT;
             }
           });

  //! Construct it with the desired level and target
  SetLogState(LogLevel l){
    obj_=NULL;
    level_= DEFAULT;
    set(l);
  }
  void set(LogLevel l) {
    reset();
    if (l != DEFAULT) {
      level_= get_log_level();
      set_log_level(l);
    } else {
      level_=DEFAULT;
    }
  }
};


IMP_END_NAMESPACE

#endif  /* IMP_SET_LOG_STATE_H */
