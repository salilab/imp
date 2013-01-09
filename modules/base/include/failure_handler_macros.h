/**
 *  \file IMP/base/failure_handler_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_FAILURE_HANDLER_MACROS_H
#define IMPBASE_FAILURE_HANDLER_MACROS_H
#include <IMP/base/base_config.h>
#include "FailureHandler.h"
#include "object_macros.h"



//! Declare a IMP::FailureHandler
/** In addition to the standard methods it declares:
    - IMP::FailureHandler::handle_failure()
*/
#define IMP_FAILURE_HANDLER(Name)               \
  IMP_IMPLEMENT(void handle_failure());         \
  IMP_OBJECT(Name)


#endif  /* IMPBASE_FAILURE_HANDLER_MACROS_H */
