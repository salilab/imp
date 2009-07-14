/**
 *  \file FailureHandler.h     \brief Exception definitions and assertions.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_FAILURE_HANDLER_H
#define IMP_FAILURE_HANDLER_H

#include "Object.h"
#include "exception.h"


IMP_BEGIN_NAMESPACE


/** A class to handle actions to take on check failures.
    The function is called when an assertion or check fails.
    \ingroup assert
 */
class IMPEXPORT FailureHandler: public Object {
 public:
  FailureHandler();
  virtual void handle_failure()=0;
  IMP_REF_COUNTED_DESTRUCTOR(FailureHandler);
};



//! Add a custom function to be called on an error
/** \relatesalso FailureHandler
    \ingroup assert
*/
IMPEXPORT void add_failure_handler(FailureHandler *f);

IMP_END_NAMESPACE

#endif  /* IMP_FAILURE_HANDLER_H */
