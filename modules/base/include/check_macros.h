/**
 *  \file IMP/base/check_macros.h
 *  \brief Exception definitions and assertions.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CHECK_MACROS_H
#define IMPBASE_CHECK_MACROS_H

#include <IMP/base/base_config.h>
#include "exception.h"
#include "compiler_macros.h"
#include <iostream>
#include <cmath>

#if !defined(IMP_HAS_CHECKS)
#error "IMP_HAS_CHECKS is not defined, compilation is broken"
#endif

#if !defined(IMP_NONE)
#error "IMP_NONE is not defined, compilation is broken"
#endif

/** Catch any IMP exception thrown by expr and terminate with an
    error message. Use this for basic error handling in main functions
    in C++. Do not use within the \imp library.
*/
#define IMP_CATCH_AND_TERMINATE(expr)                   \
  try {                                                 \
    expr;                                               \
  } catch (const IMP::base::Exception &e) {             \
    std::cerr << "Application terminated with error :"  \
              << e.what() << std::endl;                 \
    exit(1);                                            \
  }



//! Throw an exception with a message
/** The exception thrown must inherit from Exception and not be
    UsageException or InternalException as those are reserved for
    disableable checks (the IMP_INTERNAL_CHECK() and IMP_USAGE_CHECK()
    macros).
    \code
    IMP_THROW("Could not open file " << file_name,
              IOException);
    \endcode
 */
#define IMP_THROW(message, exception_name)do {                          \
    /* to bring in exceptions for backward compat */                    \
    using namespace IMP::base;                                          \
    std::ostringstream imp_throw_oss;                                   \
    imp_throw_oss << message << std::endl;                              \
  BOOST_STATIC_ASSERT((!(boost::is_base_of<IMP::base::UsageException,   \
                          exception_name>::value)                       \
                       && !(boost::is_base_of<IMP::base::InternalException, \
                             exception_name>::value)                    \
                       && (boost::is_base_of<IMP::base::Exception,      \
                            exception_name>::value)));                  \
  throw exception_name(imp_throw_oss.str().c_str());                    \
  } while (true)


//! Throw an exception if a check fails
/** Do IMP_THROW() if the check as the first argument fails. Unlike
    IMP_USAGE_CHECK() and IMP_INTERNAL_CHECK() these checks are
    always present.*/
#define IMP_ALWAYS_CHECK(condition, message, exception_name)    \
  if (!(condition)) {                                           \
    IMP_THROW(message, exception_name);                         \
  }



//! A runtime failure for IMP.
/** \param[in] message Failure message to write.
    This macro is used to provide nice error messages when there is
    an internal error in \imp. It causes an IMP::InternalException to be
    thrown.
*/
#define IMP_FAILURE(message) do {                                       \
    std::ostringstream imp_failure_oss;                                 \
    imp_failure_oss << message << std::endl;                            \
    IMP::base::handle_error(imp_failure_oss.str().c_str());             \
    throw IMP::base::InternalException(imp_failure_oss.str().c_str());  \
  } while (true)


//! Use this to make that the method is not implemented yet
/**
 */
#define IMP_NOT_IMPLEMENTED do {                                        \
    IMP::base::handle_error("This method is not implemented.");         \
    throw IMP::base::InternalException("Not implemented");              \
  } while(true)

#ifdef IMP_DOXYGEN

//! Execute the code block if a certain level checks are on
/**
   The next code block (delimited by { }) is executed if
   get_check_level() <= level.

   For example:
    \code
    IMP_IF_CHECK(USAGE) {
        base::Vector<Particle*> testp(input.begin(), input.end());
        std::sort(testp.begin(), testp.end());
        IMP_USAGE_CHECK(std::unique(testp.begin(), testp.end()) == testp.end(),
                        "Duplicate particles found in the input list.");
    }
    \endcode
*/
#define IMP_IF_CHECK(level)


//! Only compile the code if checks are enabled
/** For example
    \code
    IMP_CHECK_CODE({
        base::Vector<Particle*> testp(input.begin(), input.end());
        std::sort(testp.begin(), testp.end());
        IMP_USAGE_CHECK(std::unique(testp.begin(), testp.end()) == testp.end(),
                        "Duplicate particles found in the input list.");
    });
    \endcode
**/
#define IMP_CHECK_CODE(expr)


/** \brief An assertion to check for internal errors in \imp. An
    IMP::ErrorException will be thrown.

    Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.

    For example:
    \code
    IMP_INTERNAL_CHECK((3.14-PI) < .01,
                       "PI is not close to 3.14. It is instead " << PI);
    \endcode

    \note if the code is compiled with 'fast', or the check level is
    less than IMP::USAGE_AND_INTERNAL, the check is not performed.  Do
    not use asserts as a shorthand to throw exceptions (throw the
    exception yourself); use them only to check for logic errors.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
*/
#define IMP_INTERNAL_CHECK(expr, message)


/** This is like IMP_INTERNAL_CHECK, however designed to check if
    two floating point numbers are almost equal. The check looks something
    like
    \code
    std::abs(a-b) < .1*(a+b)+.1
    \endcode
    Using this makes such tests a bit easier to spot and not mess up.
*/
#define IMP_INTERNAL_CHECK_FLOAT_EQUAL(expra, exprb, message)

//! A runtime test for incorrect usage of a class or method.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.

    It should be used to check arguments to function. For example
    \code
    IMP_USAGE_CHECK(positive_argument >0,
                    "Argument positive_argument to function my_function "
                    << " must be positive. Instead got " << positive_argument);
    \endcode

    \note if the build is 'fast', or the check level
    is less than IMP::USAGE, the check is not performed. Do not use these
    checks as a shorthand to throw necessary exceptions (throw the
    exception yourself); use them only to check for errors, such as
    inappropriate input.
 */
#define IMP_USAGE_CHECK(expr, message)

/** This is like IMP_USAGE_CHECK, however designed to check if
    two floating point numbers are almost equal. The check looks something
    like
    \code
    std::abs(a-b) < .1*(a+b)+.1
    \endcode
    Using this makes such tests a bit easier to spot and not mess up.
*/
#define IMP_USAGE_CHECK_FLOAT_EQUAL(expra, exprb, message)

#ifndef IMP_DOXYGEN
/** Mark a variable as one that is only used in checks. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_CHECK_VARIABLE(variable)
#endif

/** Mark a variable as one that is only used in checks. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_USAGE_CHECK_VARIABLE(variable)

/** Mark a variable as one that is only used in checks. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_INTERNAL_CHECK_VARIABLE(variable)


#else // IMP_DOXYGEN


#if IMP_HAS_CHECKS == IMP_INTERNAL
#define IMP_CHECK_VARIABLE(variable)
#define IMP_USAGE_CHECK_VARIABLE(variable)
#define IMP_INTERNAL_CHECK_VARIABLE(variable)
#elif IMP_HAS_CHECKS == IMP_USAGE
#define IMP_CHECK_VARIABLE(variable)
#define IMP_USAGE_CHECK_VARIABLE(variable)
#define IMP_INTERNAL_CHECK_VARIABLE(variable) IMP_UNUSED(variable)
#else
#define IMP_CHECK_VARIABLE(variable) IMP_UNUSED(variable)
#define IMP_USAGE_CHECK_VARIABLE(variable) IMP_UNUSED(variable)
#define IMP_INTERNAL_CHECK_VARIABLE(variable) IMP_UNUSED(variable)
#endif


#if IMP_HAS_CHECKS > IMP_NONE
#define IMP_IF_CHECK(level)                      \
  using IMP::base::NONE;                         \
  using IMP::base::USAGE;                        \
  using IMP::base::USAGE_AND_INTERNAL;           \
  if (level <= ::IMP::base::get_check_level())

#define IMP_CHECK_CODE(expr) expr

#if IMP_BASE_HAS_LOG4CXX
#define IMP_BASE_CONTEXT
#else
#define IMP_BASE_CONTEXT << IMP::base::get_context_message()
#endif



#else // IMP_HAS_CHECKS == IMP_NONE
#define IMP_IF_CHECK(level) if (0)
#define IMP_CHECK_CODE(expr)
#endif // IMP_HAS_CHECKS

#if IMP_HAS_CHECKS >= IMP_INTERNAL
#define IMP_INTERNAL_CHECK(expr, message)                               \
  do {                                                                  \
    if (IMP::base::get_check_level()                                    \
        >= IMP::base::USAGE_AND_INTERNAL && !(expr)) {                  \
      std::ostringstream imp_check_oss;                                 \
      imp_check_oss << "Internal check failure: " << message << std::endl \
                   << "  File \"" << __FILE__ << "\", line " << __LINE__ \
        IMP_BASE_CONTEXT                                                \
                   << std::endl;                                        \
      IMP::base::handle_error(imp_check_oss.str().c_str());              \
      throw IMP::base::InternalException(imp_check_oss.str().c_str());   \
    }                                                                   \
  } while(false)

#define IMP_INTERNAL_CHECK_FLOAT_EQUAL(expra, exprb, message)           \
  IMP_INTERNAL_CHECK(std::abs((expra)-(exprb)) <                        \
                     .1*std::abs((expra)+(exprb))+.1,                   \
                     (expra) << " != " << (exprb)                       \
                     << " - " << message)
#else
#define IMP_INTERNAL_CHECK(expr, message)
#define IMP_INTERNAL_CHECK_FLOAT_EQUAL(expra, exprb, message)
#endif

#if IMP_HAS_CHECKS >= IMP_USAGE
#define IMP_USAGE_CHECK(expr, message)                                  \
  do {                                                                  \
    if (IMP::base::get_check_level() >= IMP::base::USAGE && !(expr)) {  \
      std::ostringstream imp_check_oss;                                 \
      imp_check_oss << "Usage check failure: " << message               \
        IMP_BASE_CONTEXT                                                \
                    << std::endl;                                       \
      IMP::base::handle_error(imp_check_oss.str().c_str());             \
      throw IMP::base::UsageException(imp_check_oss.str().c_str());     \
    }                                                                   \
  } while (false)
#define IMP_USAGE_CHECK_FLOAT_EQUAL(expra, exprb, message)              \
  IMP_USAGE_CHECK(std::abs((expra)-(exprb))                             \
                  < .1*std::abs((expra)+(exprb))+.1,                    \
                  expra << " != " << exprb                              \
                  <<" - " <<  message)
#else
#define IMP_USAGE_CHECK(expr, message)
#define IMP_USAGE_CHECK_FLOAT_EQUAL(expra, exprb, message)
#endif

#endif // IMP_DOXYGEN

#if defined(IMP_DOXYGEN) || IMP_HAS_CHECKS == IMP_NONE
//! Perform some basic validity checks on the object for memory debugging
#define IMP_CHECK_OBJECT(obj) IMP_UNUSED(obj)
#define IMP_CHECK_OBJECT_IF_NOT_nullptr(obj) IMP_UNUSED(obj)
#else

#define IMP_CHECK_OBJECT(obj) do {                                      \
    IMP_UNUSED(obj);                                                    \
    IMP_INTERNAL_CHECK((obj), "nullptr object");                           \
    IMP_INTERNAL_CHECK((obj)->get_is_valid(), "Check object "           \
                       << static_cast<const void*>(obj)                 \
                       << " was previously freed");                     \
} while (false)

#define IMP_CHECK_OBJECT_IF_NOT_nullptr(obj) do {                          \
    if (obj) {                                                          \
      IMP_INTERNAL_CHECK((obj)->get_is_valid(), "Check object "         \
                         << static_cast<const void*>(obj)               \
                         << " was previously freed");                   \
    }                                                                   \
  } while (false)
#endif


#endif  /* IMPBASE_CHECK_MACROS_H */
