/**
 *  \file IMP/kernel/optimizer_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_MACROS_H
#define IMPKERNEL_OPTIMIZER_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/value_macros.h>
#include "Optimizer.h"

/** \deprecated_at{2.1} Declare the methods directly.
*/
#define IMP_OPTIMIZER(Name)                                            \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Expand methods directly instead."); \
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;      \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPKERNEL_OPTIMIZER_MACROS_H */
