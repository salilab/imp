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


//! Define the basic things you need for an Optimizer.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Optimizer::do_optimize()

    \relatesalso IMP::Optimizer
*/
#define IMP_OPTIMIZER(Name)                                             \
  IMP_IMPLEMENT(virtual Float do_optimize(unsigned int max_steps));     \
  IMP_OBJECT(Name)



#endif  /* IMPKERNEL_OPTIMIZER_MACROS_H */
