/**
 *  \file IMP/SetCheckState.h
 *  \brief Checking and error reporting support.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SET_CHECK_STATE_H
#define IMPKERNEL_SET_CHECK_STATE_H

#include <IMP/kernel_config.h>
#include "enums.h"
#include "raii_macros.h"
#include "value_macros.h"
#include <IMP/nullptr.h>

IMPKERNEL_BEGIN_NAMESPACE
class Object;

//! A class to change and restore check state
/**
   To use, create an instance of this class with the check level you
   want. When it goes out of scope, it will restore the old level.

   \note This will not keep objects alive, make sure there is also some
   other ref-counted pointer to them.

   \ingroup checking
 */
class IMPKERNELEXPORT SetCheckState : public RAII {
  CheckLevel level_;
  Object *obj_;
  void do_set(Object *o, CheckLevel l);
  void do_reset();
  void do_show(std::ostream &out) const;

 public:
  IMP_RAII(SetCheckState, (Object *o, CheckLevel l), {
                                                       level_ = DEFAULT_CHECK;
                                                       obj_ = nullptr;
                                                     },
  { do_set(o, l); }, { do_reset(); }, do_show(out););

  //! Construct it with the desired level and target
  SetCheckState(CheckLevel l) {
    obj_ = nullptr;
    level_ = DEFAULT_CHECK;
    set(l);
  }
  void set(CheckLevel l);
};

IMP_VALUES(SetCheckState, SetCheckStates);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SET_CHECK_STATE_H */
