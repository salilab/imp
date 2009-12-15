/**
 *  \file log.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_LOG_H
#define IMP_LOG_H

#include "config.h"
#include "internal/log_internal.h"
#include "utility.h"
#include "macros.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>
#include <map>

IMP_BEGIN_NAMESPACE

/** \defgroup logging Logging

    IMP provides tools for controlling the amount of log output produced
    and directing it to the terminal or a file. Only log messages tagged
    with a lower level than the current LogLevel are emitted. In addition
    to a global local level (get_log_level(), set_log_level()), each
    IMP::Object has an internal log level (IMP::Object::get_log_level(),
    IMP::Object::set_log_level()) which is used when executing code on
    that object.

    Logging is provided by IMP/log.h.

    People implementing IMP::Object classes should also see IMP_OBJECT_LOG()
    and IMP::SetLogState.

    All logging is disabled when \imp is build using \c build='fast'.
    @{
 */

//! The log levels supported by IMP
/**
    DEFAULT is only local logging (like in IMP::Object), it means use
    the global log level

    VERBOSE prints very large amounts of information. It should be enough
    to allow the computational flow to be understood.

    TERSE prints a few lines per restraint or per state each time
    the score is evaluated.

    WARNING prints only warnings.

    MEMORY print information about allocations and deallocations to debug
    memory issues
 */
enum LogLevel {DEFAULT=-1, SILENT=0, WARNING=1, TERSE=2, VERBOSE=3,
               MEMORY=4
#ifndef IMP_DOXYGEN
               , ALL_LOG
#endif
};

//! The targets for IMP logging
enum LogTarget {COUT, FILE, CERR};


#if !defined SWIG && !defined(IMP_DOXYGEN)
namespace internal {
  IMPEXPORT extern LogLevel log_level;
  IMPEXPORT extern unsigned int log_indent;
}
#endif

//! Write a string to the log
IMPEXPORT void log_write(std::string to_write);

//! Set the current log level for IMP
IMPEXPORT void set_log_level(LogLevel l);

//! Set the target of logs
IMPEXPORT void set_log_target(LogTarget l);

//! Get the current log level for IMP
inline LogLevel get_log_level()
{
  return internal::log_level;
}

//! Set the file name for the IMP log; must be called if a file is to be used.
IMPEXPORT void set_log_file(std::string l);

#ifndef IMP_DOXYGEN
inline bool is_log_output(LogLevel l)
{
  return l <= get_log_level();
}
#endif


#if IMP_BUILD < IMP_FAST


//! Execute the code block if a certain level logging is on
/**
   The next code block (delimited by { }) is executed if
   get_check_level() <= level.
 */
#define IMP_IF_LOG(level)\
  if (level <= ::IMP::get_log_level())


//! Write an entry to a log.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr A stream expression to be sent to the output stream
 */
#define IMP_LOG(level, expr) if (IMP::is_log_output(level)) \
    { std::ostringstream oss;                               \
      oss<< expr << std::flush;                             \
      IMP::log_write(oss.str());                            \
    };

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM
 */
#define IMP_LOG_WRITE(level, expr) if (IMP::is_log_output(level)) \
    {std::ostringstream IMP_STREAM;                               \
      expr;                                                       \
      IMP::log_write(IMP_STREAM.str());                           \
    }

#else
#define IMP_LOG(l,e)
#define IMP_LOG_WRITE(l,e)
#define IMP_IF_LOG(l) if (false)
#endif



//! Write a warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
 */
#define IMP_WARN(expr) if (IMP::is_log_output(IMP::WARNING)) \
    { std::ostringstream oss;                                \
      oss << "WARNING  " << expr << std::flush;              \
      IMP::log_write(oss.str());                             \
    };


struct WarningContext {
  mutable std::map<std::string, int> data_;
public:
  void add_warning(std::string str) const {
    if (data_.find(str) == data_.end()) {
      data_[str]=1;
    } else {
      ++data_[str];
    }
  }
  void dump_warnings() const {
    for (std::map<std::string, int>::const_iterator it= data_.begin();
         it != data_.end(); ++it) {
      IMP_WARN(it->first << "(" << it->second << " times)" << std::endl);
    }
    data_.clear();
  }
  ~WarningContext() {
    dump_warnings();
  }
};


//! Write a warning once per context object
/** Use this macro to, for example, warn on unprocessable fields in a PDB,
    since they tend to come together.

    Warnings are only output when the context object is destroyed.
 */
#define IMP_WARN_ONCE(expr, context) {                       \
    std::ostringstream oss;                                  \
    oss << expr << std::flush;                               \
    context.add_warning(oss.str());                          \
  }


//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
 */
#define IMP_WARN_WRITE(expr) if (IMP::is_log_output(IMP::WARNING)) \
    {std::ostringstream IMP_STREAM;                                \
      expr;                                                        \
      IMP::log_write(IMP_STREAM.str());                            \
    }



//! Write a warning to standard error.
/** \param[in] expr An expression to be output to std::cerr. It is prefixed
                    by "ERROR"
 */
#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

//! Write an entry to standard error; for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "ERROR"
 */
#define IMP_ERROR_WRITE(expr) {         \
  std::ostream &IMP_STREAM = std::cerr; \
  std::cerr<< "ERROR ";                 \
  expr;                                 \
  std::cerr << std::endl;               \
}


//! Set the log level
/**
 */
#define IMP_SET_LOG_LEVEL(level) IMP::Log::get()::set_level(level);



//! Increase the current indent in the log by one level
struct IncreaseIndent: public RAII {
  IncreaseIndent(){
    internal::log_indent+=2;
  }
  ~IncreaseIndent() {
    internal::log_indent-=2;
  }
};

/** @} */

IMP_END_NAMESPACE

#endif  /* IMP_LOG_H */
