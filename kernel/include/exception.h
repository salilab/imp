/**
 *  \file exception.h     \brief Exception definitions and assertions.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_EXCEPTION_H
#define IMP_EXCEPTION_H

#include "config.h"

#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <new>
#include <sstream>

IMP_BEGIN_NAMESPACE


/**
    \defgroup assert Error checking and reporting

    By default \imp performs a variety of runtime error checks. These
    can be controlled using the IMP::set_check_level function. Call
    IMP::set_check_level with IMP::NONE to disable all checks when you
    are performing your optimization as opposed to testing your
    code. Make sure you run your code with the level set to at least
    CHEAP before running your final optimization to make sure that
    \imp is used correctly.

    Error handling is provided by IMP/exception.h,

    Use the \c gdbinit file provided in tools to automatically have \c gdb
    break when errors are detected.
    @{
 */

//! The general base class for IMP exceptions
/** This way we can catch IMP exceptions without getting memory allocation
    errors and everything. And it enforces having a description.
*/
class IMPEXPORT Exception
{
  struct refstring {
    char message_[256];
    int ct_;
  };
  refstring *str_;
 public:
  const char *what() const throw() {
    return str_? str_->message_: NULL;
  }
  Exception(const char *message) {
    str_= new (std::nothrow) refstring();
    if (str_ != NULL) {
      str_->ct_=0;
      std::strncpy(str_->message_, message, 255);
      str_->message_[255]='\0';
    }
  }
  /** \note By making the destructor virtual and providing an implementation in
      each derived class, we force a strong definition of the exception object
      in the kernel DSO. This allows exceptions to be passed between DSOs.

      \todo Should probably have a macro for exception classes to make sure this
      is always done correctly.
  */
  virtual ~Exception() throw();

  Exception(const Exception &o) {copy(o);}
  Exception &operator=(const Exception &o) {
    destroy();
    copy(o);
    return *this;
  }
 private:
  void destroy() {
    if (str_ != NULL) {
      --str_->ct_;
      if (str_->ct_==0) delete str_;
    }
  }
  void copy(const Exception &o) {
    str_=o.str_;
    if (str_!= NULL) ++str_->ct_;
  }
};

//! Determine the level of runtime checks performed
/** - NONE means that minimial checks are used.
    - USAGE means that checks of input values to functions
    and classes are verified.
    - USAGE_AND_INTERNAL adds checks that \imp itself is
    correct. Turn these on if you suspect an \imp bug or are
    developing Restraints or other \imp classes.
*/
enum CheckLevel {NONE=0, USAGE=1, USAGE_AND_INTERNAL=2};

//! Determine the maximum check level that can be used for this build
/** For example, 'fast' builds can't used any checks.
 */
IMPEXPORT CheckLevel get_maximum_check_level();

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  IMPEXPORT extern CheckLevel check_mode;
}
#endif


//! Control runtime checks in the code
/** The default level of checks is USAGE for release builds and
    USAGE_AND_INTERNAL for debug builds.
*/
inline void set_check_level(CheckLevel tf) {
  internal::check_mode= tf;
}

//! Get the current audit mode
/**
 */
inline CheckLevel get_check_level() {
  return internal::check_mode;
}


//! Set whether exception messages are printed or not
/** By default the error message associated with thrown exceptions are printed
    when using IMP from C++, but not from Python (since the error messages of
    unhandled exception are printed by the python runtime).
*/
IMPEXPORT void set_print_exceptions(bool tf);

#if IMP_BUILD < IMP_FAST
//! Execute the code block if a certain level checks are on
/**
   The next code block (delimited by { }) is executed if
   get_check_level() <= level.
*/
#define IMP_IF_CHECK(level)                     \
  if (level <= ::IMP::get_check_level())
#else
#define IMP_IF_CHECK(level) if (0)
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
IMPEXPORT void assert_fail(const char *msg);
}
#endif


#if IMP_BUILD < IMP_FAST

/** \brief An assertion to check for internal errors in \imp. An
    IMP::ErrorException will be thrown.

    Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.

    \note if the code is compiled with 'fast', or the check level is
    less than IMP::USAGE_AND_INTERNAL, the check is not performed.  Do
    not use asserts as a shorthand to throw exceptions (throw the
    exception yourself); use them only to check for logic errors.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
*/
#define IMP_INTERNAL_CHECK(expr, message)                       \
  do {                                                          \
    if (IMP::get_check_level() >= IMP::USAGE_AND_INTERNAL && !(expr)) { \
      std::ostringstream oss;                                   \
      oss << message << std::endl                               \
          << "  File \"" << __FILE__ << "\", line " << __LINE__ \
          << std::endl;                                         \
      IMP::internal::assert_fail(oss.str().c_str());            \
      throw IMP::InternalException(oss.str().c_str());          \
    }                                                           \
  } while(false)
#else
#define IMP_INTERNAL_CHECK(expr, message)
#endif

#if IMP_BUILD < IMP_FAST
//! A runtime test for incorrect usage of a class or method.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] ExceptionType this is now ignored and will go away.

    \note if the build is 'fast', or the check level
    is less than IMP::USAGE, the check is not performed. Do not use these
    checks as a shorthand to throw necessary exceptions (throw the
    exception yourself); use them only to check for errors, such as
    inappropriate input.

    \note The ExceptionType is ignored.
 */
#define IMP_USAGE_CHECK(expr, message, ExceptionType)           \
  do {                                                          \
    if (IMP::get_check_level() >= IMP::USAGE && !(expr)) {      \
      std::ostringstream oss;                                   \
      oss << message << std::endl;                              \
      IMP::internal::assert_fail(oss.str().c_str());            \
      throw IMP::UsageException(oss.str().c_str());             \
    }                                                           \
  } while (false)
#else
#define IMP_USAGE_CHECK(e,m,E)
#endif

//! Throw an exception with a message
#define IMP_THROW(message, exception_name)do {                          \
  std::ostringstream oss;                                               \
  oss << message << std::endl;                                          \
  throw exception_name(oss.str().c_str());                              \
  } while (true)

//! A runtime failure for IMP.
/** \param[in] message Failure message to write.
    This macro is used to provide nice error messages when there is
    an internal error in \imp. It causes an IMP::InternalException to be
    thrown.
*/
#define IMP_FAILURE(message) do {                                       \
  std::ostringstream oss;                                               \
  oss << message << std::endl;                                          \
  IMP::internal::assert_fail(oss.str().c_str());                        \
  throw InternalException(oss.str().c_str());                           \
  } while (true)

//! Use this to make that the method is not implemented yet
/**
 */
#define IMP_NOT_IMPLEMENTED do {                                        \
    IMP::internal::assert_fail("This method is not implemented.");      \
    throw InternalException("Not implemented");                         \
  } while(true)


/** @} */



//! A general exception for an intenal error in IMP.
/** This exception is thrown by the IMP_INTERNAL_CHECK() and
    IMP_FAILURE() macros. It should never be caught.
 */
struct IMPEXPORT InternalException: public Exception
{
  InternalException(const char *msg="Fatal error"): Exception(msg){}
  ~InternalException() throw();
};

//! An exception for an invalid usage of \imp
/** It is thrown by the IMP_USAGE_CHECK() macro. It should never be
    caught internally to \imp, but it one may be able to recover from
    it being thrown.
 */
class IMPEXPORT UsageException : public Exception
{
 public:
  UsageException(const char *t): Exception(t){}
  ~UsageException() throw();
};

//! An exception for an invalid value being passed to \imp
/** This exception will be passed back to python as a value error.
 */
class IMPEXPORT ValueException : public Exception
{
 public:
  ValueException(const char *t): Exception(t){}
  ~ValueException() throw();
};


//! An exception for a request for an invalid member of a container
/** This exception should be used when a python index error is
    to be thrown.
 */
class IMPEXPORT IndexException: public Exception
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
 */
class IMPEXPORT IOException: public Exception
{
 public:
  IOException(const char *t): Exception(t){}
  ~IOException() throw();
};


/** \brief An exception which is thrown when the Model has
    attributes with invalid values.

    It may be OK to catch a in \imp ModelException, when, for example,
    the catcher can simply re-randomize the optimized coordinates and
    restart the optimization.  */
class IMPEXPORT ModelException: public Exception
{
 public:
  //! Create exception with an error message
  ModelException(const char *t): Exception(t){}
  ~ModelException() throw();
};

IMP_END_NAMESPACE

#endif  /* IMP_EXCEPTION_H */
