/**
 *  \file IMP/log_macros.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_LOG_MACROS_H
#define IMPKERNEL_LOG_MACROS_H

#include <IMP/kernel_config.h>
#include "enums.h"
#include "log.h"
#include "CreateLogContext.h"
#include "compiler_macros.h"
#include "SetCheckState.h"
#include "internal/log.h"
#include <sstream>

#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/logger.h>
#endif

#if !defined(IMP_HAS_LOG)
#error "IMP_HAS_LOG is not defined, compilation is broken"
#endif

#if !defined(IMP_SILENT)
#error "IMP_SILENT is not defined, compilation is broken"
#endif

#if defined(IMP_DOXYGEN)
//! Execute the code block if a certain level of logging is on
/**
   The next code block (delimited by { }) is executed if
   get_log_level() >= level.

   \code
   IMP_IF_LOG(VERBOSE) {
     Floats testp(input.begin(), input.end());
     std::sort(testp.begin(), testp.end());
     IMP_LOG_VERBOSE( "Sorted order is ");
     IMP_LOG_WRITE(VERBOSE, std::copy(testp.begin(), testp.end(),
                   std::ostream_iterator<double>(IMP_STREAM, " ")));
   }
   \endcode
 */
#define IMP_IF_LOG(level)

//! Write a warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
 */
#define IMP_WARN(expr)

//! Write a warning to standard error.
/** \param[in] expr An expression to be output to std::cerr. It is prefixed
                    by "ERROR"
 */
#define IMP_ERROR(expr)

/** \param[in] expr A stream expression to be sent to the output stream if the
    log level is at least TERSE.

    Usage:
    \code
    IMP_LOG_VERBOSE( "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
*/
#define IMP_LOG_TERSE(expr)

/** \param[in] expr A stream expression to be sent to the output stream if the
    log level is at least VERBOSE.

    Usage:
    \code
    IMP_LOG_VERBOSE( "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
*/
#define IMP_LOG_VERBOSE(expr)

/** \param[in] expr A stream expression to be sent to the output stream if the
    log level is at least PROGRESS.

    Usage:
    \code
    IMP_LOG_PROGRESS( "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
*/
#define IMP_LOG_PROGRESS(expr)

/** Mark a variable as one that is only used in logging. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_LOG_VARIABLE(variable)

#else  // IMP_DOXYGEN

#define IMP_LOG(level, expr)                                                  \
  {                                                                           \
    switch (level) {                                                          \
      case IMP::SILENT:                                                       \
        break;                                                                \
      case IMP::PROGRESS:                                                     \
        IMP_LOG_PROGRESS(expr);                                               \
        break;                                                                \
      case IMP::TERSE:                                                        \
        IMP_LOG_TERSE(expr);                                                  \
        break;                                                                \
      case IMP::WARNING:                                                      \
        IMP_WARN(expr);                                                       \
        break;                                                                \
      case IMP::VERBOSE:                                                      \
        IMP_LOG_VERBOSE(expr);                                                \
        break;                                                                \
      case IMP::MEMORY:                                                       \
        IMP_LOG_MEMORY(expr);                                                 \
        break;                                                                \
      case IMP::DEFAULT:                                                \
      case IMP::ALL_LOG:                                                \
      default:                                                                \
        IMP_ERROR(                                                            \
            "Unknown log level " << boost::lexical_cast<std::string>(level)); \
    }                                                                         \
  }

#if IMP_HAS_LOG < IMP_PROGRESS
#define IMP_IF_LOG(level) if (false)
#define IMP_LOG_PROGRESS(expr)
#define IMP_WARN(expr) \
  if (false) std::cout << expr;
#define IMP_LOG_VARIABLE(variable) IMP_UNUSED(variable)
#else
#define IMP_LOG_VARIABLE(variable)
#endif

#if IMP_HAS_LOG < IMP_TERSE
#define IMP_LOG_TERSE(expr)
#endif

#if IMP_HAS_LOG < IMP_VERBOSE
#define IMP_LOG_VERBOSE(expr)
#define IMP_LOG_MEMORY(expr)
#endif

#if IMP_KERNEL_HAS_LOG4CXX

// figure out later

#if IMP_HAS_LOG >= IMP_PROGRESS
#define IMP_IF_LOG(level) if (true)
#define IMP_LOG_PROGRESS(expr)                   \
  {                                              \
    using IMP::internal::log::operator<<;  \
    LOG4CXX_INFO(IMP::get_logger(), expr); \
  }
#define IMP_WARN(expr)                           \
  {                                              \
    using IMP::internal::log::operator<<;  \
    LOG4CXX_WARN(IMP::get_logger(), expr); \
  }
#endif

#if IMP_HAS_LOG >= IMP_TERSE
#define IMP_LOG_TERSE(expr)                      \
  {                                              \
    using IMP::internal::log::operator<<;  \
    LOG4CXX_INFO(IMP::get_logger(), expr); \
  }
#endif

#if IMP_HAS_LOG >= IMP_VERBOSE
#define IMP_LOG_VERBOSE(expr)                     \
  {                                               \
    using IMP::internal::log::operator<<;   \
    LOG4CXX_DEBUG(IMP::get_logger(), expr); \
  }

#define IMP_LOG_MEMORY(expr)                      \
  {                                               \
    using IMP::internal::log::operator<<;   \
    LOG4CXX_TRACE(IMP::get_logger(), expr); \
  }
#endif

#define IMP_ERROR(expr)                                        \
  {                                                            \
    using IMP::internal::log::operator<<;                \
    LOG4CXX_ERROR(IMP::get_logger(), expr << std::endl); \
  }

#else  // log4cxx

#if IMP_HAS_LOG > IMP_SILENT
#define IMP_IF_LOG(level) \
  if (level <= ::IMP::get_log_level())
#define IMP_LOG_PROGRESS(expr)                             \
  if (IMP::get_log_level() >= IMP::PROGRESS) { \
    std::ostringstream oss;                                \
    oss << expr;                                           \
    IMP::add_to_log(oss.str());                      \
  }

#define IMP_WARN(expr)                                    \
  if (IMP::get_log_level() >= IMP::WARNING) { \
    std::ostringstream oss;                               \
    oss << "WARNING  " << expr << std::flush;             \
    IMP::add_to_log(oss.str());                     \
  };
#endif

#if IMP_HAS_LOG >= IMP_TERSE
#define IMP_LOG_TERSE(expr)                             \
  if (IMP::get_log_level() >= IMP::TERSE) { \
    std::ostringstream oss;                             \
    oss << expr;                                        \
    IMP::add_to_log(oss.str());                   \
  }

#endif

#if IMP_HAS_LOG >= IMP_VERBOSE
#define IMP_LOG_VERBOSE(expr)                             \
  if (IMP::get_log_level() >= IMP::VERBOSE) { \
    std::ostringstream oss;                               \
    oss << expr;                                          \
    IMP::add_to_log(oss.str());                     \
  }
#define IMP_LOG_MEMORY(expr)                             \
  if (IMP::get_log_level() >= IMP::MEMORY) { \
    std::ostringstream oss;                              \
    oss << expr;                                         \
    IMP::add_to_log(oss.str());                    \
  }
#endif

#define IMP_ERROR(expr)                                    \
  {                                                        \
    std::cerr << "ERROR: " << expr << std::endl;           \
    std::ostringstream oss;                                \
    oss << expr;                                           \
    throw IMP::InternalException(oss.str().c_str()); \
  }

#endif  // log4cxx

#endif  // else on IMP_DXOYGEN

#define IMP_ERROR_WRITE(expr)      \
  {                                \
    std::ostringstream IMP_STREAM; \
    expr;                          \
    IMP_STREAM << std::endl;       \
    IMP_ERROR(IMP_STREAM.str());   \
  }

#define IMP_LOG_WRITE(level, expr)    \
  IMP_IF_LOG(level) {                 \
    std::ostringstream IMP_STREAM;    \
    expr;                             \
    IMP_STREAM << std::endl;          \
    IMP_LOG(level, IMP_STREAM.str()); \
  }

#define IMP_WARN_WRITE(expr)       \
  IMP_IF_LOG(WARNING) {            \
    std::ostringstream IMP_STREAM; \
    expr;                          \
    IMP_STREAM << std::endl;       \
    IMP_WARN(IMP_STREAM.str());    \
  }

#if IMP_HAS_LOG

//! Set the log level to the object's log level.
/** All non-trivial Object methods should start with this. It creates a
    RAII-style object which sets the log level to the local one,
    if appropriate, until it goes out of scope.
 */
#define IMP_OBJECT_LOG                                                   \
  IMP::SetLogState log_state_guard__(this->get_log_level());       \
  IMP::SetCheckState check_state_guard__(this->get_check_level()); \
  IMP_CHECK_OBJECT(this);                                                \
  IMP::CreateLogContext log_context__(IMP_CURRENT_FUNCTION, this)

//! Beginning logging for a non-member function
/**
 */
#define IMP_FUNCTION_LOG \
  IMP::CreateLogContext log_context__(IMP_CURRENT_FUNCTION)

//! Create a new log context from a streamed name
#define IMP_LOG_CONTEXT(name) \
  IMP::CreateLogContext imp_log_context(name, nullptr)

//! Write a warning once per context object
/** Use this macro to, for example, warn on unprocessable fields in a PDB,
    since they tend to come together. The key is what is tested
    for uniqueness, the expr is what is output.

    Warnings are only output when the context object is destroyed.
 */
#define IMP_WARN_ONCE(key, expr, context) \
  IMP_IF_LOG(WARNING) {                   \
    std::ostringstream oss;               \
    oss << expr << std::flush;            \
    context.add_warning(key, oss.str());  \
  }

/** Like IMP::set_progress_display() but you can use stream operations
    for the name.*/
#define IMP_PROGRESS_DISPLAY(name, steps)                    \
  {                                                          \
    if (IMP::get_log_level() >= IMP::PROGRESS) { \
      std::ostringstream oss;                                \
      oss << name;                                           \
      IMP::set_progress_display(oss.str(), steps);     \
    }                                                        \
  }

#else
#define IMP_OBJECT_LOG
#define IMP_FUNCTION_LOG
#define IMP_LOG_CONTEXT(name)
#define IMP_WARN_ONCE(key, expr, context)
#define IMP_PROGRESS_DISPLAY(name, steps)
#endif

#endif /* IMPKERNEL_LOG_MACROS_H */
