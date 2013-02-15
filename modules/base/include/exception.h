/**
 *  \file IMP/base/exception.h
 *  \brief Exception definitions and assertions.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_EXCEPTION_H
#define IMPBASE_EXCEPTION_H

#include <IMP/base/base_config.h>
#include "random.h"
#include "enums.h"
#include <IMP/base/nullptr.h>
#include "internal/static.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/random/uniform_real.hpp>
#include <cassert>
#include <string>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>

IMPBASE_BEGIN_NAMESPACE

#ifndef IMP_DOXYGEN
typedef std::runtime_error ExceptionBase;
#endif

#ifndef SWIG
/**
    \name Error checking and reporting
    \anchor assert

    By default \imp performs a variety of runtime error checks. These
    can be controlled using the IMP::set_check_level function. Call
    IMP::set_check_level with IMP::NONE to disable all checks when you
    are performing your optimization as opposed to testing your
    code. Make sure you run your code with the level set to at least
    USAGE before running your final optimization to make sure that
    \imp is used correctly.

    Error handling is provided by IMP/exception.h,

    Use the \c gdbinit file provided in \c tools to automatically have \c gdb
    break when \imp errors are detected.
    @{
 */

//! The general base class for \imp exceptions
/** Exceptions should be used to report all errors that occur within \imp.
*/
class IMPBASEEXPORT Exception
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  : public std::runtime_error
#endif
{
 public:
#if defined(SWIG) || defined(IMP_DOXYGEN)
  const char *what() const throw();
#endif
  Exception(const char *message);
  ~Exception() throw();
};

#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN) && !IMP_BASE_HAS_LOG4CXX
IMPBASEEXPORT std::string get_context_message();
#endif


//! Control runtime checks in the code
/** The default level of checks is USAGE for release builds and
    USAGE_AND_INTERNAL for debug builds.
*/
inline void set_check_level(CheckLevel tf) {
  // cap it against the maximum supported level
  internal::check_level= std::min<int>(tf, IMP_HAS_CHECKS);
}

//! Get the current audit mode
/**
 */
inline CheckLevel get_check_level() {
#if IMP_HAS_CHECKS
  return CheckLevel(internal::check_level);
#else
  return NONE;
#endif
}



/** This function is called whenever IMP detects an error. It can be
    useful to add a breakpoint in the function when using a debugger.
*/
IMPBASEEXPORT void handle_error(const char *msg);




/** @} */

#ifndef SWIG

//! A general exception for an intenal error in IMP.
/** This exception is thrown by the IMP_INTERNAL_CHECK() and
    IMP_FAILURE() macros. It should never be caught.
 */
struct IMPBASEEXPORT InternalException
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  : public std::runtime_error
#endif
{
  InternalException(const char *msg="Fatal error"): std::runtime_error(msg){}
  ~InternalException() throw();
};

//! An exception for an invalid usage of \imp
/** It is thrown by the IMP_USAGE_CHECK() macro. It should never be
    caught internally to \imp, but it one may be able to recover from
    it being thrown.

    \advanceddoc
    As the usage checks are disabled in fast mode,
    UsageExceptions are not considered part of the API and hence
    should not be documented or checked in test cases.
 */
class IMPBASEEXPORT UsageException
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  : public std::runtime_error
#endif
{
 public:
  UsageException(const char *t): std::runtime_error(t){}
  ~UsageException() throw();
};

//! An exception for an invalid value being passed to \imp
/** The equivalent Python type also derives from Python's ValueError.
 */
class IMPBASEEXPORT ValueException : public Exception
{
 public:
  ValueException(const char *t): Exception(t){}
  ~ValueException() throw();
};


//! An exception for a request for an invalid member of a container
/** The equivalent Python type also derives from Python's IndexError.
 */
class IMPBASEEXPORT IndexException: public Exception
{
 public:
  //! Create exception with an error message
  IndexException(const char *t): Exception(t){}
  ~IndexException() throw();
};

//! An input/output exception
/** This exception should be used when an IO
    operation fails in a way that leaves the internal state OK. For
    example, failure to open a file should result in an IOException.

    It is OK to catch such exceptions in \imp.

    The equivalent Python type also derives from Python's IOError.
 */
class IMPBASEEXPORT IOException: public Exception
{
 public:
  IOException(const char *t): Exception(t){}
  ~IOException() throw();
};


/** \brief An exception which is thrown when the Model has
    attributes with invalid values.

    It may be OK to catch an \imp ModelException, when, for example,
    the catcher can simply re-randomize the optimized coordinates and
    restart the optimization. Sampling protocols, such as
    IMP::core::MCCGSampler, tend to do this.
 */
class IMPBASEEXPORT ModelException: public Exception
{
 public:
  //! Create exception with an error message
  ModelException(const char *t): Exception(t){}
  ~ModelException() throw();
};

/** \brief An exception that signifies some event occurred.

    It is difficult to add exceptions to the python wrappers,
    so use this type if want to raise an exception when something
    happens.

    We can add event types later via a key.
 */
class IMPBASEEXPORT EventException: public Exception
{
 public:
  //! Create exception with an error message
  EventException(const char *t=""): Exception(t){}
  ~EventException() throw();
};
#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_EXCEPTION_H */
