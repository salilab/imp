/**
 *  \file IMP/base/SetLogState.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SET_LOG_STATE_H
#define IMPBASE_SET_LOG_STATE_H

#include <IMP/base/base_config.h>
#include "enums.h"
#include "raii_macros.h"
#include "value_macros.h"
#include <IMP/base/nullptr.h>

IMPBASE_BEGIN_NAMESPACE
class Object;

//! A class to change and restore log state
/**
   To use, create an instance of this class with the log level you
   want. When it goes out of scope, it will restore the old level.


   \note This will not keep objects alive, make sure there is also some
   other ref-counted pointer to them.

   \ingroup logging
 */
class IMPBASEEXPORT SetLogState: public base::RAII
{
  LogLevel level_;
  Object* obj_;
  void do_set(Object *o, LogLevel l);
  void do_reset();
  void do_show(std::ostream &out) const;
public:
  IMP_RAII(SetLogState, (Object *o, LogLevel l),
           {level_= DEFAULT; obj_=nullptr;},
           {
             do_set(o, l);
           },
           {
             do_reset();
           }, do_show(out););

  //! Construct it with the desired level and target
  SetLogState(LogLevel l);
  void set(LogLevel l);
};


IMP_VALUES(SetLogState, SetLogStates);

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_SET_LOG_STATE_H */
