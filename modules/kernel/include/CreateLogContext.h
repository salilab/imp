/**
 *  \file IMP/CreateLogContext.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CREATE_LOG_CONTEXT_H
#define IMPKERNEL_CREATE_LOG_CONTEXT_H

#include <IMP/kernel_config.h>
#include "raii_macros.h"
#include "log.h"
#include <IMP/nullptr.h>
#include "Object.h"
#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/ndc.h>
#include <boost/scoped_ptr.hpp>
#endif

IMPKERNEL_BEGIN_NAMESPACE

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
            IMP_LOG_VERBOSE( 1);
            IMP_LOG_VERBOSE( 2);
            IMP_LOG_VERBOSE( 3);
        }
        IMP_LOG_VERBOSE( "Now it is has ended." << std::endl);
        \endcode

        The more interesting use is that you can use it before
        calling a function to ensure that all the output of that
        function is nicely offset.
    */
class CreateLogContext : public RAII {
#if IMP_KERNEL_HAS_LOG4CXX
  log4cxx::NDC ndc0_;
  boost::scoped_ptr<log4cxx::NDC> ndc1_;

 public:
  CreateLogContext(const char* fname, const Object* object)
      : ndc0_(object->get_quoted_name_c_string()),
        ndc1_(new log4cxx::NDC(fname)) {}
  CreateLogContext(const char* fname) : ndc0_(fname) {}
  CreateLogContext(std::string name) : ndc0_(name) {}
  IMP_SHOWABLE_INLINE(CreateLogContext, IMP_UNUSED(out));
#else
  bool pushed_;
  std::string name_;

 public:
  CreateLogContext(std::string fname, const Object* object = nullptr)
      : pushed_(true), name_(fname) {
    // push log context does not copy the string, so we need to save it.
    push_log_context(name_.c_str(), object);
  }
  IMP_RAII(CreateLogContext,
           (const char* fname, const Object* object = nullptr), ,
           { push_log_context(fname, object);
             pushed_ = true; },
           { if (pushed_) pop_log_context();
             pushed_ = false; }, );
#endif
};

/** @} */

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CREATE_LOG_CONTEXT_H */
