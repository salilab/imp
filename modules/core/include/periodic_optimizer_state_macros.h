/**
 *  \file periodic_optimizer_state_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H
#define IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H

#include "PeriodicOptimizerState.h"

/** Declares
    - IMP::core::PeriodicOptimizerState::do_update()
    as well as the IMP_OBJECT() declarations.
 */
#define IMP_CORE_PERIODIC_OPTIMIZER_STATE(Name)                         \
  IMP_IMPLEMENT(virtual void do_update(unsigned int num));              \
  IMP_OBJECT(Name)


#endif  /* IMPCORE_PERIODIC_OPTIMIZER_STATE_MACROS_H */
