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

/** Do not use, simply use IMP_OBJECT_METHODS() and declare
    - IMP::core::PeriodicOptimizerState::do_update()
 */
#define IMP_CORE_PERIODIC_OPTIMIZER_STATE(Name)                         \
  /** do a periodic update of Name

      @param call_num the number of times do_update() was called
                      since creation of Name or since last call to
                      reset()
  */                                                                    \
  IMP_IMPLEMENT(virtual void do_update(unsigned int call_num));         \
  IMP_OBJECT(Name)


#endif  /* IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H */
