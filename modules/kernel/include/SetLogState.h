/**
 *  \file IMP/SetLogState.h     \brief A class to change and restore log state
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SET_LOG_STATE_H
#define IMPKERNEL_SET_LOG_STATE_H

#include <IMP/kernel_config.h>
#include "enums.h"
#include "raii_macros.h"
#include "value_macros.h"
#include <IMP/nullptr.h>

IMPKERNEL_BEGIN_NAMESPACE
class Object;

//! A class to change and restore log state
/**
   To use, create an instance of this class with the log level you
   want. When it goes out of scope, it will restore the old level.


   \note This will not keep objects alive; make sure there is also some
   other ref-counted pointer to them.

   \ingroup logging
 */
class IMPKERNELEXPORT SetLogState : public RAII {
  LogLevel level_;
  Object *obj_;
  void do_set(Object *o, LogLevel l);
  void do_reset();
  void do_show(std::ostream &out) const;

 public:
  IMP_RAII(SetLogState, (Object *o, LogLevel l), {
                                                   level_ = DEFAULT;
                                                   obj_ = nullptr;
                                                 },
  { do_set(o, l); }, { do_reset(); }, do_show(out););

  //! Construct with the desired level and target
  SetLogState(LogLevel l);
  void set(LogLevel l);
};

IMP_VALUES(SetLogState, SetLogStates);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SET_LOG_STATE_H */
