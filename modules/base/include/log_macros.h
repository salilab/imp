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
#include "SetCheckState.h"
#include "internal/log.h"
#include <sstream>

#if IMP_BASE_HAS_LOG4CXX
#  include <log4cxx/logger.h>
#endif


#ifndef IMP_DOXYGEN
#define IMP_LOG_USING                                     \
  using IMP::base::VERBOSE;                               \
  using IMP::base::TERSE;                                 \
  using IMP::base::SILENT;                                \
  using IMP::base::WARNING;                               \
  using IMP::base::PROGRESS;                              \
  using IMP::base::MEMORY;                                \
  using IMP::base::INFO;                                  \
  using IMP::base::TRACE;
#endif

#if defined(IMP_DOXYGEN) || IMP_BUILD == IMP_FAST
//! Execute the code block if a certain level of logging is on
/**
   The next code block (delimited by { }) is executed if
   get_log_level() >= level.

   \code
   IMP_IF_LOG(VERBOSE) {
     Floats testp(input.begin(), input.end());
     std::sort(testp.begin(), testp.end());
     IMP_LOG(VERBOSE, "Sorted order is ");
     IMP_LOG_WRITE(VERBOSE, std::copy(testp.begin(), testp.end(),
                   std::ostream_iterator<double>(IMP_STREAM, " ")));
   }
   \endcode
 */
#define IMP_IF_LOG(level)

//! Write an entry to a log.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr A stream expression to be sent to the output stream

    Usage:
    \code
    IMP_LOG(VERBOSE, "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
 */
#define IMP_LOG(level, expr)

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


#elif IMP_BASE_HAS_LOG4CXX

// figure out later
#define IMP_IF_LOG(level) if (true)

#define IMP_LOG(level, expr)                                            \
  {                                                                     \
    using IMP::base::internal::log::operator<<;                          \
    IMP_LOG_USING;                                                      \
    switch(level) {                                                     \
    case SILENT:                                                        \
      break;                                                            \
    case PROGRESS:                                                      \
    case TERSE:                                                         \
      LOG4CXX_INFO(IMP::base::get_logger(), expr);                      \
      break;                                                            \
    case WARNING:                                                       \
      LOG4CXX_WARN(IMP::base::get_logger(), expr);                      \
      break;                                                            \
    case VERBOSE:                                                       \
      LOG4CXX_TRACE(IMP::base::get_logger(), expr);                     \
      break;                                                            \
    case MEMORY:                                                        \
      /* not supported */                                               \
      break;                                                            \
    default:                                                            \
      IMP_ERROR("Unknown log level "                                    \
                << boost::lexical_cast<std::string>(level));            \
    }                                                                   \
  }

#define IMP_WARN(expr) {                                                \
    using IMP::base::internal::log::operator<<;                          \
    LOG4CXX_WARN(IMP::base::get_logger(), expr);                        \
}

#define IMP_ERROR(expr) {                                               \
    using IMP::base::internal::log::operator<<;                          \
    LOG4CXX_ERROR(IMP::base::get_logger(), expr);                       \
  }


#else

#define IMP_IF_LOG(level)                               \
  IMP_LOG_USING;                                        \
  if (level <= ::IMP::base::get_log_level())

#define IMP_LOG(level, expr)                                            \
  {                                                                     \
    IMP_LOG_USING;                                                      \
    if (IMP::base::get_is_log_output(level)){                           \
      std::ostringstream oss;                                           \
      oss<< expr << std::flush;                                         \
      IMP::base::add_to_log(oss.str());                                 \
    }                                                                   \
  }

#define IMP_WARN(expr) if (IMP::base::get_is_log_output(IMP::base::WARNING)) \
    { std::ostringstream oss;                                \
      oss << "WARNING  " << expr << std::flush;              \
      IMP::base::add_to_log(oss.str());                      \
    };

#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

#endif // else on IMP_DXOYGEN

//! Write an entry to standard error; for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "ERROR"
 */
#define IMP_ERROR_WRITE(expr) {         \
    std::ostringstream IMP_STREAM;           \
    expr;                                    \
    IMP_STREAM << std::endl;                 \
    IMP_ERROR(IMP_STREAM.str());             \
}

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM

    \code
    IMP_LOG_WRITE(VERBOSE, IMP::atom::write_pdb(h, IMP_STREAM));
    \endcode
 */
#define IMP_LOG_WRITE(level, expr)                                      \
  IMP_IF_LOG(level) {                                                   \
    std::ostringstream IMP_STREAM;                                      \
    expr;                                                               \
    IMP_STREAM << std::endl;                                            \
    IMP_LOG(level, IMP_STREAM.str());                                   \
  }

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
 */
#define IMP_WARN_WRITE(expr)                                       \
  IMP_IF_LOG(WARNING) {                                                 \
    std::ostringstream IMP_STREAM;                                      \
    expr;                                                               \
    IMP_STREAM << std::endl;                                            \
    IMP_WARN(IMP_STREAM.str());                                         \
  }

#if IMP_BUILD < IMP_FAST

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

#else
#define IMP_OBJECT_LOG
#define IMP_FUNCTION_LOG
#define IMP_LOG_CONTEXT(name)
#define IMP_WARN_ONCE(key, expr, context)
#endif

#endif  /* IMPBASE_LOG_MACROS_H */
