/**
 *  \file IMP/exception.h
 *  \brief Exception definitions and assertions.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_EXCEPTION_H
#define IMPKERNEL_EXCEPTION_H

#include <IMP/kernel_config.h>
#include "compiler_macros.h"
#include "enums.h"
#include "nullptr.h"
#include "internal/base_static.h"
#include <string>
#include <stdexcept>

IMPKERNEL_BEGIN_NAMESPACE

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
class IMPKERNELEXPORT Exception
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    : public std::runtime_error
#endif
      {
 public:
#if defined(SWIG) || defined(IMP_DOXYGEN)
  const char *what() const IMP_NOEXCEPT;
#endif
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(Exception);
  Exception(const char *message);
  ~Exception() IMP_NOEXCEPT;
};

#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN) && !IMP_KERNEL_HAS_LOG4CXX
IMPKERNELEXPORT std::string get_context_message();
#endif

//! Control runtime checks in the code
/** The default level of checks is USAGE for release builds and
    USAGE_AND_INTERNAL for debug builds.
*/
inline void set_check_level(CheckLevel tf) {
  // cap it against the maximum supported level
  internal::check_level = std::min<CheckLevel>(tf, CheckLevel(IMP_HAS_CHECKS));
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
IMPKERNELEXPORT void handle_error(const char *msg);

/** @} */

#ifndef SWIG

//! A general exception for an internal error in IMP.
/** This exception is thrown by the IMP_INTERNAL_CHECK() and
    IMP_FAILURE() macros. It should never be caught.
 */
struct IMPKERNELEXPORT InternalException
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    : public std::runtime_error
#endif
      {
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(InternalException);
  InternalException(const char *msg = "Fatal error")
      : std::runtime_error(msg) {}
  ~InternalException() IMP_NOEXCEPT;
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
class IMPKERNELEXPORT UsageException
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    : public std::runtime_error
#endif
      {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(UsageException);
  UsageException(const char *t) : std::runtime_error(t) {}
  ~UsageException() IMP_NOEXCEPT;
};

//! An exception for an invalid value being passed to \imp
/** The equivalent Python type also derives from Python's ValueError.
 */
class IMPKERNELEXPORT ValueException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(ValueException);
  ValueException(const char *t) : Exception(t) {}
  ~ValueException() IMP_NOEXCEPT;
};

//! An exception for an invalid type being passed to \imp
/** The equivalent Python type also derives from Python's TypeError.
 */
class IMPKERNELEXPORT TypeException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(TypeException);
  TypeException(const char *t) : Exception(t) {}
  ~TypeException() IMP_NOEXCEPT;
};

//! An exception for a request for an invalid member of a container
/** The equivalent Python type also derives from Python's IndexError.
 */
class IMPKERNELEXPORT IndexException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(IndexException);
  //! Create exception with an error message
  IndexException(const char *t) : Exception(t) {}
  ~IndexException() IMP_NOEXCEPT;
};

//! An input/output exception
/** This exception should be used when an IO
    operation fails in a way that leaves the internal state OK. For
    example, failure to open a file should result in an IOException.

    It is OK to catch such exceptions in \imp.

    The equivalent Python type also derives from Python's IOError.
 */
class IMPKERNELEXPORT IOException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(IOException);
  IOException(const char *t) : Exception(t) {}
  ~IOException() IMP_NOEXCEPT;
};

/** \brief An exception which is thrown when the Model has
    attributes with invalid values.

    It may be OK to catch an \imp ModelException, when, for example,
    the catcher can simply re-randomize the optimized coordinates and
    restart the optimization. Sampling protocols, such as
    IMP::core::MCCGSampler, tend to do this.
 */
class IMPKERNELEXPORT ModelException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(ModelException);
  //! Create exception with an error message
  ModelException(const char *t) : Exception(t) {}
  ~ModelException() IMP_NOEXCEPT;
};

//! An exception that signifies some event occurred.
/** It is difficult to add exceptions to the Python wrappers,
    so use this type if want to raise an exception when something
    happens.

    We can add event types later via a key.
 */
class IMPKERNELEXPORT EventException : public Exception {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(EventException);
  //! Create exception with an error message
  EventException(const char *t = "") : Exception(t) {}
  ~EventException() IMP_NOEXCEPT;
};
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_EXCEPTION_H */
