/**
 *  \file log.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_LOG_H
#define IMP_LOG_H

#include "kernel_config.h"
#include "macros.h"
#include <boost/timer.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>
#include <map>

IMP_BEGIN_NAMESPACE

/** \name Logging
    \anchor log
    \imp provides tools for controlling the amount of log output produced
    and directing it to the terminal or a file. Only log messages tagged
    with a lower level than the current LogLevel are emitted. In addition
    to a global log level (get_log_level(), set_log_level()), each
    IMP::Object has an internal log level (IMP::Object::get_log_level(),
    IMP::Object::set_log_level()) which is used when executing code on
    that object.

    Logging is provided by IMP/log.h.

    People implementing IMP::Object classes should also see IMP_OBJECT_LOG()
    and IMP::SetLogState.

    All logging is disabled when \imp is built using \c build='fast'.
    @{
 */

//! The log levels supported by \imp
/**
    DEFAULT is only local logging (like in IMP::Object); it means to use
    the global log level.

    VERBOSE prints very large amounts of information. It should be enough
    to allow the computational flow to be understood.

    TERSE prints a few lines per restraint or per state each time
    the score is evaluated.

    PROGRESS show how large calculations are progressing (eg samplers)
    but otherwise just show warnings

    WARNING prints only warnings.

    MEMORY prints information about allocations and deallocations to debug
    memory issues.
 */
enum LogLevel {DEFAULT=-1, SILENT=0, WARNING=1, PROGRESS=2,
               TERSE=3, VERBOSE=4,
               MEMORY=5
#ifndef IMP_DOXYGEN
               , ALL_LOG
#endif
};


#if !defined SWIG && !defined(IMP_DOXYGEN)
namespace internal {
  IMPEXPORT extern LogLevel log_level;
  IMPEXPORT extern unsigned int log_indent;
  IMPEXPORT extern bool print_time;
  IMPEXPORT extern boost::timer log_timer;
}
#endif

//! Push a new log context onto the stack
/** A log context is, eg, a function name.
 */
IMPEXPORT void push_log_context(std::string name);

//! pop the top log context
IMPEXPORT void pop_log_context();

//! Write a string to the log
IMPEXPORT void add_to_log(std::string to_write);


//! Set the current global log level
/** Note that this should not, currently, be used directly
    during Model::evaluate() calls. */
IMPEXPORT void set_log_level(LogLevel l);

//! Set whether log messages are tagged with the current log time
IMPEXPORT void set_log_timer(bool tb);

//! Reset the log timer
IMPEXPORT void reset_log_timer();


//! Get the currently active log level
/** This may not always match the value passed to set_log_level()
    as objects can temporarily override the global level
    while they are evaluating.
 */
inline LogLevel get_log_level()
{
  return internal::log_level;
}

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline bool get_is_log_output(LogLevel l)
{
  return l <= get_log_level();
}
#endif

#ifdef IMP_DOXYGEN
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
#define IMP_IF_LOG(level)\
  if (level <= ::IMP::get_log_level())


//! Write an entry to a log.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr A stream expression to be sent to the output stream

    Usage:
    \code
    IMP_LOG(VERBOSE, "Hi there, I'm very talkative. My favorite numbers are "
                     << 1 << " " << 2 << " " << 3);
    \endcode
 */
#define IMP_LOG(level, expr) if (IMP::get_is_log_output(level)) \
    { std::ostringstream oss;                               \
      oss<< expr << std::flush;                             \
      IMP::add_to_log(oss.str());                           \
    };

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM

    \code
    IMP_LOG_WRITE(VERBOSE, IMP::atom::write_pdb(h, IMP_STREAM));
    \endcode
 */
#define IMP_LOG_WRITE(level, expr) if (IMP::get_is_log_output(level)) \
    {std::ostringstream IMP_STREAM;                               \
      expr;                                                       \
      IMP::add_to_log(IMP_STREAM.str());                          \
    }
#else // IMP_DOXYGEN

#if IMP_BUILD < IMP_FAST

#define IMP_IF_LOG(level)\
  if (level <= ::IMP::get_log_level())

#define IMP_LOG(level, expr) if (IMP::get_is_log_output(level)) \
    { std::ostringstream oss;                               \
      oss<< expr << std::flush;                             \
      IMP::add_to_log(oss.str());                           \
    };

#define IMP_LOG_WRITE(level, expr) if (IMP::get_is_log_output(level)) \
    {std::ostringstream IMP_STREAM;                               \
      expr;                                                       \
      IMP::add_to_log(IMP_STREAM.str());                          \
    }

#else
#define IMP_LOG(l,e)
#define IMP_LOG_WRITE(l,e)
#define IMP_IF_LOG(l) if (false)
#endif
#endif // else on IMP_DXOYGEN


#ifdef IMP_DOXYGEN
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

#elif IMP_BUILD==IMP_FAST

#define IMP_WARN(expr)


struct WarningContext {
public:
  void add_warning(std::string ) const {}
  void clear_warnings() const {}
  void dump_warnings() const {}
  void show(std::ostream &) const {}
};


#define IMP_WARN_ONCE(key, expr, context)


#define IMP_WARN_WRITE(expr)

#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

#define IMP_ERROR_WRITE(expr)



#else
#define IMP_WARN(expr) if (IMP::get_is_log_output(IMP::WARNING)) \
    { std::ostringstream oss;                                \
      oss << "WARNING  " << expr << std::flush;              \
      IMP::add_to_log(oss.str());                            \
    };


struct WarningContext {
  mutable std::map<std::string, std::string> data_;
public:
  void add_warning(std::string key, std::string warning) const {
    if (warning.empty()) return;
    if (IMP::get_is_log_output(IMP::WARNING)) {
      if (data_.find(key) == data_.end()) {
        data_[key]=warning;
      }
    }
  }
  void clear_warnings() const {
    data_.clear();
  }
  void dump_warnings() const {
    for (std::map<std::string, std::string>::iterator it= data_.begin();
         it != data_.end(); ++it) {
      if (!it->second.empty()) {
        IMP_WARN(it->second << std::endl);
        it->second=std::string();
      }
    }
  }
  ~WarningContext() {
    dump_warnings();
  }
  IMP_SHOWABLE_INLINE(WarningContext, out << data_.size() << " warnings");
};


#define IMP_WARN_ONCE(key, expr, context) {                  \
    std::ostringstream oss;                                  \
    oss << expr << std::flush;                               \
    context.add_warning(key, oss.str());                     \
  }


#define IMP_WARN_WRITE(expr) if (IMP::get_is_log_output(IMP::WARNING)) \
    {std::ostringstream IMP_STREAM;                                \
      expr;                                                        \
      IMP::add_to_log(IMP_STREAM.str());                            \
    }

#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

#define IMP_ERROR_WRITE(expr) {         \
  std::ostream &IMP_STREAM = std::cerr; \
  std::cerr<< "ERROR ";                 \
  expr;                                 \
  std::cerr << std::endl;               \
}
#endif


//! Create a new log context
/** The following produces

    \verbatim
    myfunction:
       1
       2
       3
    \endverbatim

    \code
    {
        CreateLogContext ii("myfunction ");
        IMP_LOG(VERBOSE, 1);
        IMP_LOG(VERBOSE, 2);
        IMP_LOG(VERBOSE, 3);
    }
    IMP_LOG(VERBOSE, "Now it is has ended." << std::endl);
    \endcode

    The more interesting use is that you can use it before
    calling a function to ensure that all the output of that
    function is nicely offset.
*/
struct CreateLogContext {
public:
  IMP_RAII(CreateLogContext, (std::string name),,
           push_log_context(name),
           pop_log_context(),);
};

//! Create a new long context from a streamed name
#define IMP_LOG_CONTEXT(name)                                           \
  std::ostringstream imp_log_context_stream;                            \
  imp_log_context_stream << name;                                       \
  CreateLogContext imp_log_context(imp_log_context_stream.str())

/** @} */

IMP_END_NAMESPACE

#endif  /* IMP_LOG_H */
