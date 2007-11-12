/**
 *  \file log.h     Logging and error reporting support.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
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
enum Log_Level {SILENT=0, WARNING=1, TERSE=2, VERBOSE=3};

//! The targets for IMP logging
enum Log_Target {COUT, FILE, CERR};

namespace internal {

class IMPDLLEXPORT Log
{
public:

  Log_Level get_level() {
    return level_;
  }
  void set_level(Log_Level l) {
    level_=l;
  }

  std::ostream &get_stream(Log_Level l) {
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

  bool is_output(Log_Level l) {
    return l <= get_level();
  }

  Log_Target get_target() {
    return target_;
  }
  void set_target(Log_Target k) {
    target_=k;
  }
  void set_filename(std::string k) {
    fstream_.open(k.c_str());
    if (!fstream_) {
      std::cerr << "Error opening log file " << k << std::endl;
    }
  };


  Log()  :level_(SILENT), target_(COUT) {
    static int count=0;
    ++count;
    assert(count==1);
  }
private:

  Log(const Log&) {}

  Log_Level level_;
  Log_Target target_;
  std::ofstream fstream_;
};

}

//! A general exception for an error in IMP
class IMPDLLEXPORT ErrorException: public std::exception {
};

//! An exception for an invalid model state
class IMPDLLEXPORT InvalidStateException : public std::exception {
public:
  //! just eat the string for now
  template <class T>
  InvalidStateException(T){}
  InvalidStateException(){}
};

//! An exception for a request for an invalid member of a container
class IMPDLLEXPORT IndexException: public std::exception {
public:
  //! just eat the string for now
  template <class T>
  IndexException(T){}
  IndexException(){}
};

//! Get the log object. This really shouldn't be called from C++.
IMPDLLEXPORT inline internal::Log &get_log()
{
  static internal::Log l;
  return l;
}

//! Set the current log level for IMP
IMPDLLEXPORT inline void set_log_level(Log_Level l)
{
  get_log().set_level(l);
}

//! Set the target of logs
IMPDLLEXPORT inline void set_log_target(Log_Target l)
{
  get_log().set_target(l);
}

//! Set the file name for the IMP log; must be called if a file is to be used.
IMPDLLEXPORT inline void set_log_file(std::string l)
{
  get_log().set_filename(l);
}

//! Determine whether a given log level should be output.
/** \note This probably should not be called in C++.
 */
IMPDLLEXPORT inline bool is_log_output(Log_Level l)
{
  return get_log().is_output(l);
}


//! The stream to output a particular log level to.
/** \note This probably should not be called in C++.
 */
IMPDLLEXPORT inline std::ostream& get_log_stream(Log_Level l)
{
  return get_log().get_stream(l);
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


//! Write an warning to a log.
/** \param[in] expr An expression to be output to the log. It is prefixed
                    by "WARNING"
 */
#define IMP_WARN(expr) if (IMP::get_log_output(IMP::WARNING)) \
    { IMP::get_log_stream(IMP::WARNING) << "WARNING  " << expr << std::flush;};

//! Write an entry to a log. This is to be used for objects with no operator<<.
/** \param[in] expr An expression which writes something to IMP_STREAM.
                    It is prefixed by "WARNING"
 */
#define IMP_WARN_WRITE(expr) if (IMP::is_log_output(IMP::WARNING)) \
    {std::ostream &IMP_STREAM= IMP::get_log_stream(IMP::Log::WARNING); expr;}



//! Write an warning to a standard error.
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
#define IMP_SET_LOG_LEVEL(level) IMP::Log::get_log()::set_level(level);



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
    throw IMP::ErrorException(); \
  }
#else
#define IMP_assert(expr, message)
#endif

//! An runtime check for IMP.
/** \param[in] expr The assertion expression.
    \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
 */
#define IMP_check(expr, message, exception) \
  if (!(expr)) { \
    IMP_ERROR(message); \
    throw exception; \
  }

//! An runtime failure for IMP.
/** \param[in] message Write this message if the assertion fails.
    \param[in] exception Throw the object constructed by this expression.
 */
#define IMP_failure(message, exception) {IMP_ERROR(message); throw exception;}

#endif  /* __IMP_LOG_H */
