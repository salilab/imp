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
/** NONE means that minimial checks are used. CHEAP
    means that only constant time checks are performed
    and with EXPENSIVE non-linear time checks will be run.
*/
enum CheckLevel {NONE=0, CHEAP=1, EXPENSIVE=2};


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  IMPEXPORT extern CheckLevel check_mode;
}
#endif


//! Control runtime checks in the code
/** The default level of checks is CHEAP.
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

#ifndef NDEBUG
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


#ifndef NDEBUG

/** \brief An assertion to check for internal errors in \imp. An
    IMP::ErrorException will be thrown.

    Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.
    \note if the code is compiled with NDEBUG, or the check level is less than
    EXPENSIVE, the check is not performed. Do not use asserts as a shorthand
    to throw exceptions (throw the exception yourself); use them only to
    check for logic errors.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
*/
#define IMP_assert(expr, message)                               \
  do {                                                          \
    if (IMP::get_check_level() >= IMP::EXPENSIVE && !(expr)) {  \
      std::ostringstream oss;                                   \
      oss << message << std::endl                               \
          << "  File \"" << __FILE__ << "\", line " << __LINE__ \
          << std::endl;                                         \
      IMP::internal::assert_fail(oss.str().c_str());            \
      throw ErrorException(oss.str().c_str());                  \
    }                                                           \
  } while(false)
#else
#define IMP_assert(expr, message)
#endif

#ifndef NDEBUG
//! A runtime test for incorrect usage of a class or method.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] ExceptionType Throw an exception of this type. The exception
    must be constructable from a char *.
    \note if the code is compiled with NDEBUG, or the check level is less than
    CHEAP, the check is not performed. Do not use these checks as a shorthand
    to throw necessary exceptions (throw the exception yourself); use them
    only to check for errors, such as inappropriate input.
*/
#define IMP_check(expr, message, ExceptionType)                 \
  do {                                                          \
    if (IMP::get_check_level() >= IMP::CHEAP && !(expr)) {      \
      std::ostringstream oss;                                   \
      oss << message << std::endl;                              \
      IMP::internal::assert_fail(oss.str().c_str());            \
      throw ExceptionType(oss.str().c_str());                   \
    }                                                           \
  } while (false)
#else
#define IMP_check(e,m,E)
#endif

//! A runtime failure for IMP.
/** \param[in] message Failure message to write.
    \param[in] ExceptionType Throw an exception of this type. The exception
    must be constructable from a char *.
*/
#define IMP_failure(message, ExceptionType) do {                        \
  std::ostringstream oss;                                               \
  oss << message << std::endl;                                          \
  IMP::internal::assert_fail(oss.str().c_str());                        \
  throw ExceptionType(oss.str().c_str());                               \
  } while (true)

//! Use this to make that the method is not implemented yet
/**
 */
#define IMP_not_implemented do {                                        \
    IMP::internal::assert_fail("This method is not implemented.");      \
    throw ErrorException("Not implemented");                            \
  } while(true)


/** @} */



//! A general exception for an error in IMP.
/**
 */
struct IMPEXPORT ErrorException: public Exception
{
  //! Create exception with an error message
  ErrorException(const char *msg="Fatal error"): Exception(msg){}
  ~ErrorException() throw();
};

//! An exception for an invalid model state
/**
 */
class IMPEXPORT InvalidStateException : public Exception
{
 public:
  //! Create exception with an error message
  InvalidStateException(const char *t): Exception(t){}
  ~InvalidStateException() throw();
};

//! An exception for trying to access an inactive particle
/**
 */
class IMPEXPORT InactiveParticleException : public Exception
{
 public:
  //! Create exception with an error message
  InactiveParticleException(const char *msg
                            ="Attempting to use inactive particle"):
    Exception(msg){}
  ~InactiveParticleException() throw();
};

//! An exception for a request for an invalid member of a container
/**
 */
class IMPEXPORT IndexException: public Exception
{
 public:
  //! Create exception with an error message
  IndexException(const char *t): Exception(t){}
  ~IndexException() throw();
};

//! An exception for a passing an out of range value
/**
 */
class IMPEXPORT ValueException: public Exception
{
 public:
  //! Create exception with an error message
  ValueException(const char *t): Exception(t){}
  ~ValueException() throw();
};

//! An input/output exception
/**
 */
class IMPEXPORT IOException: public Exception
{
 public:
  //! Create exception with an error message
  IOException(const char *t): Exception(t){}
  ~IOException() throw();
};

IMP_END_NAMESPACE

#endif  /* IMP_EXCEPTION_H */
