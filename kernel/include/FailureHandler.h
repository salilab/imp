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
    Implementors should see IMP_FAILURE_HANDLER().
 */
class IMPEXPORT FailureHandler: public Object {
 public:
  FailureHandler();
  virtual void handle_failure()=0;
  IMP_REF_COUNTED_DESTRUCTOR(FailureHandler);
};


//! A base class to setting up failure handler
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
IMPEXPORT void add_failure_handler(FailureHandler *f);

//! Remove a failure handler from the list
/** \relatesalso FailureHandler
    \ingroup assert
*/
IMPEXPORT void remove_failure_handler(FailureHandler *f);


//! Control a scope-dependent failure handler
/** The failure handler is added on construction and removed
    on destruction.
*/
class ScopedFailureHandler: public RAII {
  FailureHandler* fh_;
public:
  IMP_RAII(ScopedFailureHandler, (FailureHandler *fh),
           {fh_=NULL;},
           {
             fh_=fh;
             if (fh_) add_failure_handler(fh_);
           },
           {
             if (fh_) remove_failure_handler(fh_);
             fh_=NULL;
           }
           )
};

IMP_END_NAMESPACE

#endif  /* IMP_FAILURE_HANDLER_H */
