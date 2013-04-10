/**
 *  \file IMP/base/log_macros.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_LOG_MACROS_H
#define IMPBASE_LOG_MACROS_H

#include <IMP/base/base_config.h>
#include "enums.h"
#include "log.h"
#include "CreateLogContext.h"
#include "compiler_macros.h"
#include "SetCheckState.h"
#include "internal/log.h"
#include <sstream>

#if IMP_BASE_HAS_LOG4CXX
  #include <log4cxx/logger.h>
#endif


#ifndef IMP_DOXYGEN
#define IMP_LOG_USING                                     \
  using IMP::base::VERBOSE;                               \
  using IMP::base::TERSE;                                 \
  using IMP::base::SILENT;                                \
  using IMP::base::WARNING;                               \
  using IMP::base::PROGRESS;                              \
  using IMP::base::MEMORY

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
    log level is at least TERSE.

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
    IMP_LOG_VERBOSE( "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
*/
#define IMP_LOG_PROGRESS(expr)

/** Mark a variable as one that is only used in logging. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_LOG_VARIABLE(variable)


#else // IMP_DOXYGEN

#define IMP_LOG(level, expr)                                            \
  {                                                                     \
    IMP_LOG_USING;                                                      \
    switch(level) {                                                     \
    case SILENT:                                                        \
      break;                                                            \
    case PROGRESS:                                                      \
      IMP_LOG_PROGRESS(expr); break;\
    case TERSE:                                                         \
      IMP_LOG_TERSE(expr); break;\
    case WARNING:                                                       \
      IMP_WARN(expr); break;\
    case VERBOSE:                                                       \
      IMP_LOG_VERBOSE(expr); break;\
    case MEMORY:                                                        \
      IMP_LOG_MEMORY(expr); break;\
    case IMP::base::DEFAULT:                                            \
    case IMP::base::ALL_LOG:                                            \
    default:                                                            \
      IMP_ERROR("Unknown log level "                                    \
                << boost::lexical_cast<std::string>(level));            \
    }                                                                   \
  }


#if IMP_HAS_LOG < IMP_PROGRESS
#define IMP_IF_LOG(level) if (false)
#define IMP_LOG_PROGRESS(expr)
#define IMP_WARN(expr) if (false) std::cout << expr;
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

#if IMP_BASE_HAS_LOG4CXX

// figure out later

#if IMP_HAS_LOG >= IMP_PROGRESS
#define IMP_IF_LOG(level) if (true)
#define IMP_LOG_PROGRESS(expr) {\
  using IMP::base::internal::log::operator<<; \
  LOG4CXX_INFO(IMP::base::get_logger(), expr);\
  }
#define IMP_WARN(expr) {                                               \
    using IMP::base::internal::log::operator<<;                        \
    LOG4CXX_WARN(IMP::base::get_logger(), expr);                       \
  }
#endif

#if IMP_HAS_LOG >= IMP_TERSE
#define IMP_LOG_TERSE(expr) {                           \
    using IMP::base::internal::log::operator<<;         \
    LOG4CXX_INFO(IMP::base::get_logger(), expr);        \
  }
#endif

#if IMP_HAS_LOG >= IMP_VERBOSE
#define IMP_LOG_VERBOSE(expr) {                         \
    using IMP::base::internal::log::operator<<;         \
    LOG4CXX_DEBUG(IMP::base::get_logger(), expr);       \
  }

#define IMP_LOG_MEMORY(expr) {                          \
    using IMP::base::internal::log::operator<<;         \
    LOG4CXX_TRACE(IMP::base::get_logger(), expr);       \
  }
#endif



#define IMP_ERROR(expr) {                                               \
  using IMP::base::internal::log::operator<<;                          \
  LOG4CXX_ERROR(IMP::base::get_logger(), expr);                       \
  }


#else // log4cxx

#if IMP_HAS_LOG > IMP_SILENT
#define IMP_IF_LOG(level)                               \
  IMP_LOG_USING;                                        \
  if (level <= ::IMP::base::get_log_level())
#define IMP_LOG_PROGRESS(expr) \
    if (IMP::base::get_log_level() >= IMP::base::PROGRESS) {\
         std::ostringstream oss; oss << expr;\
         IMP::base::add_to_log(oss.str());\
    }

#define IMP_WARN(expr) if (IMP::base::get_log_level() >= IMP::base::WARNING) \
    { std::ostringstream oss;                                \
      oss << "WARNING  " << expr << std::flush;              \
      IMP::base::add_to_log(oss.str());                      \
    };
#endif

#if IMP_HAS_LOG >= IMP_TERSE
#define IMP_LOG_TERSE(expr)  \
   if (IMP::base::get_log_level() >= IMP::base::TERSE) {\
         std::ostringstream oss; oss << expr;\
         IMP::base::add_to_log(oss.str());\
    }

#endif

#if IMP_HAS_LOG >= IMP_VERBOSE
#define IMP_LOG_VERBOSE(expr) \
   if (IMP::base::get_log_level() >= IMP::base::VERBOSE) {\
         std::ostringstream oss; oss << expr;\
         IMP::base::add_to_log(oss.str());\
    }
#define IMP_LOG_MEMORY(expr) \
   if (IMP::base::get_log_level() >= IMP::base::MEMORY) {\
         std::ostringstream oss; oss << expr;\
         IMP::base::add_to_log(oss.str());\
    }
#endif

#define IMP_ERROR(expr)                                 \
    {\
  std::cerr << "ERROR: " << expr << std::endl;          \
  throw IMP::base::InternalException("Failure");\
}

#endif // log4cxx

#endif // else on IMP_DXOYGEN

#define IMP_ERROR_WRITE(expr) {         \
    std::ostringstream IMP_STREAM;           \
    expr;                                    \
    IMP_STREAM << std::endl;                 \
    IMP_ERROR(IMP_STREAM.str());             \
}

#define IMP_LOG_WRITE(level, expr)                                      \
  IMP_IF_LOG(level) {                                                   \
    std::ostringstream IMP_STREAM;                                      \
    expr;                                                               \
    IMP_STREAM << std::endl;                                            \
    IMP_LOG(level, IMP_STREAM.str());                                   \
  }

#define IMP_WARN_WRITE(expr)                                       \
  IMP_IF_LOG(WARNING) {                                                 \
    std::ostringstream IMP_STREAM;                                      \
    expr;                                                               \
    IMP_STREAM << std::endl;                                            \
    IMP_WARN(IMP_STREAM.str());                                         \
  }

#if IMP_HAS_LOG

//! Set the log level to the object's log level.
/** All non-trivial Object methods should start with this. It creates a
    RAII-style object which sets the log level to the local one,
    if appropriate, until it goes out of scope.
 */
#define IMP_OBJECT_LOG                                                  \
  IMP::base::SetLogState log_state_guard__(this->get_log_level());      \
  IMP::base::SetCheckState check_state_guard__(this->get_check_level()); \
  IMP_CHECK_OBJECT(this);                                               \
  IMP::base::CreateLogContext log_context__(__func__, this)

//! Beginning logging for a non-member function
/**
 */
#define IMP_FUNCTION_LOG                                                \
  IMP::base::CreateLogContext log_context__(__func__)

//! Create a new long context from a streamed name
#define IMP_LOG_CONTEXT(name)                                           \
  IMP::base::CreateLogContext imp_log_context(name, nullptr)

//! Write a warning once per context object
/** Use this macro to, for example, warn on unprocessable fields in a PDB,
    since they tend to come together. The key is what is tested
    for uniqueness, the expr is what is output.

    Warnings are only output when the context object is destroyed.
 */
#define IMP_WARN_ONCE(key, expr, context) IMP_IF_LOG(WARNING) { \
    std::ostringstream oss;                                     \
    oss << expr << std::flush;                                  \
    context.add_warning(key, oss.str());                        \
  }


/** Like IMP::base::set_progress_display() but you can use stream operations
    for the name.*/
#define IMP_PROGRESS_DISPLAY(name, steps) {\
  if (IMP::base::get_log_level() >= IMP::base::PROGRESS) {      \
    std::ostringstream oss;                                     \
    oss << name;                                                \
    IMP::base::set_progress_display(oss.str(), steps);          \
  }                                                             \
  }

#else
#define IMP_OBJECT_LOG
#define IMP_FUNCTION_LOG
#define IMP_LOG_CONTEXT(name)
#define IMP_WARN_ONCE(key, expr, context)
#define IMP_PROGRESS_DISPLAY(name, steps)
#endif


#endif  /* IMPBASE_LOG_MACROS_H */
