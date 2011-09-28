/**
 *  \file IMP/FailureHandler.h     \brief Handle actions on check failures.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FAILURE_HANDLER_H
#define IMPKERNEL_FAILURE_HANDLER_H

#include "kernel_config.h"
#include <IMP/base/FailureHandler.h>

IMP_BEGIN_NAMESPACE

#ifndef SWIG
using base::FailureHandler;
using base::FailureHandlerBase;
using base::add_failure_handler;
using base::remove_failure_handler;
#endif

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_FAILURE_HANDLER_H */
