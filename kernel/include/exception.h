/**
 *  \file exception.h     \brief Exception definitions and assertions.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_EXCEPTION_H
#define IMP_EXCEPTION_H

#include "log.h"
#include "config.h"

#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <new>
#include <sstream>

IMP_BEGIN_NAMESPACE

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

//! A general exception for an error in IMP.
/** \ingroup assert
 */
struct IMPEXPORT ErrorException: public Exception
{
  //! Create exception with an error message
  ErrorException(const char *msg="Fatal error"): Exception(msg){}
  ~ErrorException() throw();
};

//! An exception for an invalid model state
/** \ingroup assert
 */
class IMPEXPORT InvalidStateException : public Exception
{
public:
  //! Create exception with an error message
  InvalidStateException(const char *t): Exception(t){}
  ~InvalidStateException() throw();
};

//! An exception for trying to access an inactive particle
/** \ingroup assert
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
/** \ingroup assert
 */
class IMPEXPORT IndexException: public Exception
{
public:
  //! Create exception with an error message
  IndexException(const char *t): Exception(t){}
  ~IndexException() throw();
};

//! An exception for a passing an out of range value
/** \ingroup assert
 */
class IMPEXPORT ValueException: public Exception
{
public:
  //! Create exception with an error message
  ValueException(const char *t): Exception(t){}
  ~ValueException() throw();
};

//! Determine the level of runtime checks performed
/** NONE means that minimial checks are used. CHEAP
    means that only constant time checks are performed
    and with EXPENSIVE non-linear time checks will be run.
 */
enum CheckLevel {NONE=0, CHEAP=1, EXPENSIVE=2};


//! Control runtime checks in the code
/** The default level of checks is CHEAP.
 */
IMPEXPORT void set_check_level(CheckLevel tf);

//! Get the current audit mode
IMPEXPORT CheckLevel get_check_level();


//! Execute the code block if a certain level checks are on
/**
   The next code block (delimited by { }) is executed if
   get_check_level() <= level.
 */
#define IMP_IF_CHECK(level)\
  if (level <= ::IMP::get_check_level())

//! This is just here so you can catch errors more easily in the debugger
/** Break on exception.cpp:31 to catch assertion failures.
    \ingroup assert
 */
IMPEXPORT void assert_fail(const char *msg);

//! Here so you can catch check failures more easily in the debugger
/** Break on exception.cpp:35 to catch check failures.
    \ingroup assert
 */
IMPEXPORT void check_fail(const char *msg);

#ifndef NDEBUG

//! An assertion for IMP. An IMP::ErrorException will be thrown.
/** Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.
    \note if the code is compiled with NDEBUG, this is a noop.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \ingroup assert
 */
#define IMP_assert(expr, message)                                       \
  do {                                                                  \
    if (IMP::get_check_level() >= IMP::EXPENSIVE && !(expr)) {          \
      std::ostringstream oss;                                           \
      oss << message << std::endl                                       \
          << "  File \"" << __FILE__ << "\", line " << __LINE__         \
          << std::endl;                                                 \
      IMP::assert_fail(oss.str().c_str());                              \
    }                                                                   \
  } while(false)
#else
#define IMP_assert(expr, message)
#endif

//! A runtime check for IMP.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] ExceptionType Throw an exception of this type. The exception
    must be constructable from a char *.
    \ingroup assert
 */
#define IMP_check(expr, message, ExceptionType)                         \
  do {                                                                  \
    if (IMP::get_check_level() >= IMP::CHEAP && !(expr)) {              \
      std::ostringstream oss;                                           \
      oss << message << std::endl;                                      \
      IMP::check_fail(oss.str().c_str());                               \
      throw ExceptionType(oss.str().c_str());                           \
    }                                                                   \
  } while (false)

//! A runtime failure for IMP.
/** \param[in] message Failure message to write.
    \param[in] ExceptionType Throw an exception of this type. The exception
    must be constructable from a char *.
    \ingroup assert
 */
#define IMP_failure(message, ExceptionType) { \
    std::ostringstream oss;                                             \
    oss << message << std::endl;                                        \
    IMP::check_fail(oss.str().c_str());                                 \
    throw ExceptionType(oss.str().c_str());}

IMP_END_NAMESPACE

#endif  /* IMP_EXCEPTION_H */
