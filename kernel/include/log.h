/**
 *  \file log.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_LOG_H
#define IMP_LOG_H

#include "IMP_config.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>

IMP_BEGIN_NAMESPACE

/** \defgroup log Logging
    \defgroup assert Error checking and reporting
 */

//! The log levels supported by IMP
/** \ingroup log
    VERBOSE prints very large amounts of information. It should be enough
    to allow the computational flow to be understood.
    TERSE prints a few lines per restraint or per state each time
    the score is evaluated.
    WARNING prints only warnings.
 */
enum LogLevel {SILENT=0, WARNING=1, TERSE=2, VERBOSE=3};

//! The targets for IMP logging
/** \ingroup log
 */
enum LogTarget {COUT, FILE, CERR};

class IMPDLLEXPORT Log
{
public:
  //! Get a reference to a singleton Log object.
  static Log& get() {
    if (!logpt_) {
      logpt_ = new Log();
    }
    return *logpt_;
  }

  LogLevel get_level() {
    return level_;
  }
  void set_level(LogLevel l) {
    level_=l;
  }

  std::ostream &get_stream(LogLevel l) {
    if (is_output(l)) {
      if (target_== COUT) {
        return std::cout;
      } else if (target_== CERR) {
        return std::cerr;
      } else {
        return fstream_;
      }
    } else return std::cout;
  }

  bool is_output(LogLevel l) {
    return l <= get_level();
  }

  LogTarget get_target() {
    return target_;
  }
  void set_target(LogTarget k) {
    target_=k;
  }
  void set_filename(std::string k) {
    fstream_.open(k.c_str());
    if (!fstream_) {
      std::cerr << "Error opening log file " << k << std::endl;
    }
  }

private:

  Log()  :level_(SILENT), target_(COUT) {}
  Log(const Log&) {}

  LogLevel level_;
  LogTarget target_;
  std::ofstream fstream_;
  static Log *logpt_;
};


//! Set the current log level for IMP
/** \ingroup log
 */
IMPDLLEXPORT inline void set_log_level(LogLevel l)
{
  Log::get().set_level(l);
}

//! Set the target of logs
/** \ingroup log
 */
IMPDLLEXPORT inline void set_log_target(LogTarget l)
{
  Log::get().set_target(l);
}

//! Get the current log level for IMP
/** \ingroup log
 */
IMPDLLEXPORT inline LogLevel get_log_level()
{
  return Log::get().get_level();
}

//! Get the target of logs
/** \ingroup log
 */
IMPDLLEXPORT inline LogTarget get_log_target()
{
  return Log::get().get_target();
}

//! Set the file name for the IMP log; must be called if a file is to be used.
/** \ingroup log
 */
IMPDLLEXPORT inline void set_log_file(std::string l)
{
  Log::get().set_filename(l);
}

//! Determine whether a given log level should be output.
/** \note This probably should not be called in C++.
    \ingroup log
 */
IMPDLLEXPORT inline bool is_log_output(LogLevel l)
{
  return Log::get().is_output(l);
}


//! The stream to output a particular log level to.
/** \note This probably should not be called in C++.
    \ingroup log
 */
IMPDLLEXPORT inline std::ostream& get_log_stream(LogLevel l)
{
  return Log::get().get_stream(l);
}


//! A class to change and restore log state
/**
   To use, create an instance of this class which the log level you
   want. When it goes out of scope, it will restore the old level.
   \ingroup log
 */
class IMPDLLEXPORT SetLogState
{
public:
  //! Construct it with the desired level and target
  SetLogState(LogLevel l, LogTarget t= get_log_target()):
    level_(get_log_level()), target_(get_log_target()) {
    set_log_level(l);
    set_log_target(t);
  }
  ~SetLogState() {
    set_log_level(level_);
    set_log_target(target_);
  }
private:
  LogLevel level_;
  LogTarget target_;
};

#ifndef IMP_DISABLE_LOGGING

//! Write an entry to a log.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr A stream expression to be sent to the output stream
    \ingroup log
 */
#define IMP_LOG(level, expr) if (IMP::is_log_output(level)) \
    { IMP::get_log_stream(level) << expr << std::flush;};

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM
    \ingroup log
 */
#define IMP_LOG_WRITE(level, expr) if (IMP::is_log_output(level)) \
    {std::ostream &IMP_STREAM= IMP::get_log_stream(level); expr;}


//! Write a warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
    \ingroup log
 */
#define IMP_WARN(expr) if (IMP::is_log_output(IMP::WARNING)) \
    { IMP::get_log_stream(IMP::WARNING) << "WARNING  " << expr << std::flush;};

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
    \ingroup log
 */
#define IMP_WARN_WRITE(expr) if (IMP::is_log_output(IMP::WARNING)) \
    {std::ostream &IMP_STREAM= IMP::get_log_stream(IMP::Log::WARNING); expr;}



//! Write a warning to standard error.
/** \param[in] expr An expression to be output to std::cerr. It is prefixed
                    by "ERROR"
    \ingroup log
 */
#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

//! Write an entry to standard error; for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "ERROR"
    \ingroup log
 */
#define IMP_ERROR_WRITE(expr) { \
  std::ostream &IMP_STREAM = std::cerr; \
  std:cerr<< "ERROR "; \
  expr; \
  std::cerr << std::endl; \
}


//! Set the log level
/** \ingroup log
 */
#define IMP_SET_LOG_LEVEL(level) IMP::Log::get()::set_level(level);



#else
#define IMP_LOG(l,e)
#define IMP_LOG_WRITE(l,e)
#endif

IMP_END_NAMESPACE

#endif  /* IMP_LOG_H */
