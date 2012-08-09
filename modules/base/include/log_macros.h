/**
 *  \file IMP/base/log_macros.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_LOG_MACROS_H
#define IMPBASE_LOG_MACROS_H

#include "base_config.h"
#include "enums.h"
#include "log.h"
#include "CreateLogContext.h"
#include "SetCheckState.h"
#include <IMP/compatibility/compiler.h>

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

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM

    \code
    IMP_LOG_WRITE(VERBOSE, IMP::atom::write_pdb(h, IMP_STREAM));
    \endcode
 */
#define IMP_LOG_WRITE(level, expr)

#else // IMP_DOXYGEN

#define IMP_IF_LOG(level)                               \
  using IMP::base::VERBOSE;                             \
  using IMP::base::TERSE;                               \
  using IMP::base::SILENT;                              \
  if (level <= ::IMP::base::get_log_level())

#define IMP_LOG(level, expr)                                            \
  {                                                                     \
    using IMP::base::VERBOSE; using IMP::base::TERSE;                   \
    if (IMP::base::get_is_log_output(level)){                           \
      std::ostringstream oss;                                           \
      oss<< expr << std::flush;                                         \
      IMP::base::add_to_log(oss.str());                                 \
    }                                                                   \
  }

#define IMP_LOG_WRITE(level, expr)                                      \
  if (IMP::base::get_is_log_output(IMP::base::level))                   \
  {std::ostringstream IMP_STREAM;                                       \
    expr;                                                               \
    IMP::base::add_to_log(IMP_STREAM.str());                            \
  }

#endif // else on IMP_DXOYGEN


#if defined(IMP_DOXYGEN) || IMP_BUILD==IMP_FAST
//! Write a warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
 */
#define IMP_WARN(expr)


//! Write a warning once per context object
/** Use this macro to, for example, warn on unprocessable fields in a PDB,
    since they tend to come together. The key is what is tested
    for uniqueness, the expr is what is output.

    Warnings are only output when the context object is destroyed.
 */
#define IMP_WARN_ONCE(key, expr, context)

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
 */
#define IMP_WARN_WRITE(expr)

//! Write a warning to standard error.
/** \param[in] expr An expression to be output to std::cerr. It is prefixed
                    by "ERROR"
 */
#define IMP_ERROR(expr)

//! Write an entry to standard error; for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "ERROR"
 */
#define IMP_ERROR_WRITE(expr)

#else
#define IMP_WARN(expr) if (IMP::base::get_is_log_output(IMP::base::WARNING)) \
    { std::ostringstream oss;                                \
      oss << "WARNING  " << expr << std::flush;              \
      IMP::base::add_to_log(oss.str());                      \
    };

#define IMP_WARN_ONCE(key, expr, context) {                  \
    std::ostringstream oss;                                  \
    oss << expr << std::flush;                               \
    context.add_warning(key, oss.str());                     \
  }


#define IMP_WARN_WRITE(expr)                                       \
  if (IMP::base::get_is_log_output(IMP::base::WARNING))            \
    {std::ostringstream IMP_STREAM;                                \
      expr;                                                        \
      IMP::base::add_to_log(IMP_STREAM.str());                     \
    }

#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

#define IMP_ERROR_WRITE(expr) {         \
  std::ostream &IMP_STREAM = std::cerr; \
  std::cerr<< "ERROR ";                 \
  expr;                                 \
  std::cerr << std::endl;               \
}
#endif



#if defined(IMP_DOXYGEN) || IMP_BUILD==IMP_FAST
//! Set the log level to the object's log level.
/** All non-trivial Object methods should start with this. It creates a
    RAII-style object which sets the log level to the local one,
    if appropriate, until it goes out of scope.
 */
#define IMP_OBJECT_LOG

//! Beginning logging for a non-member function
/**
 */
#define IMP_FUNCTION_LOG

//! Create a new long context from a streamed name
#define IMP_LOG_CONTEXT(name)


#else

#define IMP_OBJECT_LOG \
  IMP::base::SetLogState log_state_guard__(this->get_log_level());      \
  IMP::base::SetCheckState check_state_guard__(this->get_check_level()); \
  IMP_CHECK_OBJECT(this);                                               \
  IMP::base::CreateLogContext log_context__(__func__, this)


#define IMP_FUNCTION_LOG                                                \
  IMP::base::CreateLogContext log_context__(__func__)

#define IMP_LOG_CONTEXT(name)                                           \
  CreateLogContext imp_log_context(name, nullptr)

#endif // fast


#endif  /* IMPBASE_LOG_MACROS_H */
