/**
 *  \file base/FailureHandler.h     \brief Handle actions on check failures.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_FAILURE_HANDLER_H
#define IMPBASE_FAILURE_HANDLER_H

#include "Object.h"
#include "Pointer.h"
#include "exception.h"
#include "VersionInfo.h"

IMPBASE_BEGIN_NAMESPACE


/** A class to handle actions to take on check failures.
    The function is called when an assertion or check fails.
    \ingroup assert
    Implementors should see IMP_FAILURE_HANDLER().
 */
class IMPBASEEXPORT FailureHandler: public Object {
 public:
  FailureHandler(std::string name="Falure Handler %1%");
  virtual void handle_failure()=0;
  IMP_REF_COUNTED_DESTRUCTOR(FailureHandler);
};


//! A base class for setting up failure handlers
/** The only function you have to implement is handle_failure.
    \untested{FailureHandlerBase}
    \unstable{FailureHandlerBase}
 */
class FailureHandlerBase: public FailureHandler {
public:
  void show(std::ostream &out=std::cout) const {
    out << "Temporary FailureHandler" << std::endl;
  }
  VersionInfo get_version_info() const {
    return VersionInfo();
  }
  IMP_REF_COUNTED_DESTRUCTOR(FailureHandlerBase);
};


//! Add a custom function to be called on an error
/** \relatesalso FailureHandler
    \ingroup assert
*/
IMPBASEEXPORT void add_failure_handler(FailureHandler *f);

//! Remove a failure handler from the list
/** \relatesalso FailureHandler
    \ingroup assert
*/
IMPBASEEXPORT void remove_failure_handler(FailureHandler *f);

IMP_OBJECTS(FailureHandler,FailureHandlers);


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_FAILURE_HANDLER_H */
