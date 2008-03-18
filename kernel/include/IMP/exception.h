/**
 *  \file exception.h     \brief Exception definitions and assertions.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_EXCEPTION_H
#define __IMP_EXCEPTION_H

#include "log.h"
#include "IMP_config.h"

#include <cassert>
#include <string>
#include <iostream>
#include <new>

namespace IMP
{
//! The general base class for IMP exceptions
/** This way we can catch IMP exceptions without getting memory allocation
    errors and everything. And it enforces having a description.
 */
class IMPDLLEXPORT Exception
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
  ~Exception() throw() {
    destroy();
  }
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
struct IMPDLLEXPORT ErrorException: public Exception
{
  ErrorException(): Exception("Fatal error"){}
};

//! An exception for an invalid model state
/** \ingroup assert
 */
class IMPDLLEXPORT InvalidStateException : public Exception
{
public:
  InvalidStateException(const char *t): Exception(t){}
};

//! An exception for trying to access an inactive particle
/** \ingroup assert
 */
class IMPDLLEXPORT InactiveParticleException : public Exception
{
public:
  InactiveParticleException(): 
    Exception("Attempting to use inactive particle"){} 
};

//! An exception for a request for an invalid member of a container
/** \ingroup assert
 */
class IMPDLLEXPORT IndexException: public Exception
{
public:
  IndexException(const char *t): Exception(t){}
};

//! An exception for a passing an out of range value
/** \ingroup assert
 */
class IMPDLLEXPORT ValueException: public Exception
{
public:
  ValueException(const char *t): Exception(t){}
};


namespace internal
{

//! This is just here so you can catch errors more easily in the debugger
/** Break on Log.cpp:19 to catch assertion failures.
    \ingroup assert
 */
IMPDLLEXPORT void assert_fail();

//! Here so you can catch check failures more easily in the debugger
/** Break on Log.cpp:22 to catch check failures.
    \ingroup assert
 */
IMPDLLEXPORT void check_fail();

} // namespace internal

} // namespace IMP


#ifndef NDEBUG

//! An assertion for IMP. An IMP::ErrorException will be thrown.
/** Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \ingroup assert
 */
#define IMP_assert(expr, message)               \
  do {                                          \
    if (!(expr)) {                              \
      IMP_ERROR(message);                       \
      IMP::internal::assert_fail();             \
    }                                           \
  } while(false)
#else
#define IMP_assert(expr, message)
#endif

//! A runtime check for IMP.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
    \ingroup assert
 */
#define IMP_check(expr, message, exception) \
  do {                                      \
    if (!(expr)) {                          \
      IMP_ERROR(message);                   \
      throw exception;                      \
    }                                       \
  } while (false)

//! A runtime failure for IMP.
/** \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
    \ingroup assert
 */
#define IMP_failure(message, exception) {IMP_ERROR(message); throw exception;}

#endif  /* __IMP_EXCEPTION_H */
