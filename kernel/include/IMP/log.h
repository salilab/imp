/**
 *  \file log.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_LOG_H
#define __IMP_LOG_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>

#include "IMP_config.h"

namespace IMP
{

//! The log levels supported by IMP
enum LogLevel {SILENT=0, WARNING=1, TERSE=2, VERBOSE=3};

//! The targets for IMP logging
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

//! A general exception for an error in IMP
class IMPDLLEXPORT ErrorException: public std::exception
{
};

//! An exception for an invalid model state
class IMPDLLEXPORT InvalidStateException : public std::exception
{
public:
  //! just eat the string for now
  template <class T>
  InvalidStateException(T){}
  InvalidStateException(){}
};

//! An exception for a request for an invalid member of a container
class IMPDLLEXPORT IndexException: public std::exception
{
public:
  //! just eat the string for now
  template <class T>
  IndexException(T){}
  IndexException(){}
};

//! Set the current log level for IMP
IMPDLLEXPORT inline void set_log_level(LogLevel l)
{
  Log::get().set_level(l);
}

//! Set the target of logs
IMPDLLEXPORT inline void set_log_target(LogTarget l)
{
  Log::get().set_target(l);
}

//! Get the current log level for IMP
IMPDLLEXPORT inline LogLevel get_log_level()
{
  return Log::get().get_level();
}

//! Get the target of logs
IMPDLLEXPORT inline LogTarget get_log_target()
{
  return Log::get().get_target();
}

//! Set the file name for the IMP log; must be called if a file is to be used.
IMPDLLEXPORT inline void set_log_file(std::string l)
{
  Log::get().set_filename(l);
}

//! Determine whether a given log level should be output.
/** \note This probably should not be called in C++.
 */
IMPDLLEXPORT inline bool is_log_output(LogLevel l)
{
  return Log::get().is_output(l);
}


//! The stream to output a particular log level to.
/** \note This probably should not be called in C++.
 */
IMPDLLEXPORT inline std::ostream& get_log_stream(LogLevel l)
{
  return Log::get().get_stream(l);
}


//! A class to change and restore log state
/**
   To use, create an instance of this class which the log level you
   want. When it goes out of scope, it will restore the old level.
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

namespace internal {
  //! This is just here so you can catch errors more easily in the debugger
  /**
     Break on Log.cpp:19 to catch assertion failures.
   */
  IMPDLLEXPORT void assert_fail();

  //! Here so you can catch check failures more easily in the debugger
  /**
     Break on Log.cpp:22 to catch check failures. 
   */
  IMPDLLEXPORT void check_fail();
}

} // namespace IMP

#ifndef IMP_DISABLE_LOGGING

//! Write an entry to a log.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr A stream expression to be sent to the output stream
 */
#define IMP_LOG(level, expr) if (IMP::is_log_output(level)) \
    { IMP::get_log_stream(level) << expr << std::flush;};

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] level The IMP::Log_Level for the message
    \param[in] expr An expression which writes something to IMP_STREAM
 */
#define IMP_LOG_WRITE(level, expr) if (IMP::is_log_output(level)) \
    {std::ostream &IMP_STREAM= IMP::get_log_stream(level); expr;}


//! Write a warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
 */
#define IMP_WARN(expr) if (IMP::is_log_output(IMP::WARNING)) \
    { IMP::get_log_stream(IMP::WARNING) << "WARNING  " << expr << std::flush;};

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
 */
#define IMP_WARN_WRITE(expr) if (IMP::is_log_output(IMP::WARNING)) \
    {std::ostream &IMP_STREAM= IMP::get_log_stream(IMP::Log::WARNING); expr;}



//! Write a warning to standard error.
/** \param[in] expr An expression to be output to std::cerr. It is prefixed
                    by "ERROR"
 */
#define IMP_ERROR(expr) std::cerr << "ERROR: " << expr << std::endl;

//! Write an entry to standard error; for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "ERROR"
 */
#define IMP_ERROR_WRITE(expr) { \
  std::ostream &IMP_STREAM = std::cerr; \
  std:cerr<< "ERROR "; \
  expr; \
  std::cerr << std::endl; \
}


//! Set the log level
#define IMP_SET_LOG_LEVEL(level) IMP::Log::get()::set_level(level);



#else
#define IMP_LOG(l,e)
#define IMP_LOG_WRITE(l,e)
#endif

#ifndef NDEBUG

//! An assertion for IMP. An IMP::ErrorException will be thrown.
/** Since it is a debug-only check and no attempt should be made to
    recover from it, the exception type cannot be specified.

    \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
 */
#define IMP_assert(expr, message) \
  if (!(expr)) { \
    IMP_ERROR(message); \
    IMP::internal::assert_fail();               \
  }
#else
#define IMP_assert(expr, message)
#endif

//! A runtime check for IMP.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
 */
#define IMP_check(expr, message, exception) \
  if (!(expr)) { \
    IMP_ERROR(message); \
    throw exception; \
  }

//! A runtime failure for IMP.
/** \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
 */
#define IMP_failure(message, exception) {IMP_ERROR(message); throw exception;}



#endif  /* __IMP_LOG_H */
