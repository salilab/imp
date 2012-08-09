/**
 *  \file IMP/base/CreateLogContext.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CREATE_LOG_CONTEXT_H
#define IMPBASE_CREATE_LOG_CONTEXT_H

#include "base_config.h"
#include "raii_macros.h"
#include "log.h"
#include <IMP/compatibility/nullptr.h>

IMPBASE_BEGIN_NAMESPACE

//! Create a new log context
/** The following produces

    \verbatim
    myfunction:
       1
       2
       3
    \endverbatim

    \code
    {
        CreateLogContext ii("myfunction ");
        IMP_LOG(VERBOSE, 1);
        IMP_LOG(VERBOSE, 2);
        IMP_LOG(VERBOSE, 3);
    }
    IMP_LOG(VERBOSE, "Now it is has ended." << std::endl);
    \endcode

    The more interesting use is that you can use it before
    calling a function to ensure that all the output of that
    function is nicely offset.
*/
struct CreateLogContext: public RAII {
  std::string name_;
public:
  CreateLogContext(std::string fname, const void* object=nullptr):
      name_(fname) {
    // push log context does not copy the string, so we need to save it.
    push_log_context(name_.c_str(), object);
  }
  IMP_RAII(CreateLogContext, (const char *fname, const void* object=nullptr),,
           push_log_context(fname, object),
           pop_log_context(),);
};

/** @} */

IMPBASE_END_NAMESPACE


#endif  /* IMPBASE_CREATE_LOG_CONTEXT_H */
