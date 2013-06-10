/**
 *  \file IMP/core/periodic_optimizer_state_macros.h
 *  \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H
#define IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H

#include "PeriodicOptimizerState.h"

IMPCORE_DEPRECATED_HEADER(2.1, "Not needed any more")

/** Do not use, simply use IMP_OBJECT_METHODS() and declare
    - IMP::core::PeriodicOptimizerState::do_update()
*/
#define IMP_CORE_PERIODIC_OPTIMIZER_STATE(Name)                         \
  IMP_DEPRECATED_MACRO(2.1, "Just expand the macro in place instead");        \
  /** do a periodic update of Name                                      \
                                                                        \
             @param call_num the number of times do_update() was called \
             since creation of Name or since last call to               \
             reset()                                                    \
  */                                                                    \
  virtual void do_update(unsigned int call_num) IMP_OVERRIDE;           \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H */
