/**
 *  \file IMP/log.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_LOG_H
#define IMPKERNEL_LOG_H

#include <IMP/kernel_config.h>
#include "enums.h"
#include "WarningContext.h"
#include "Flag.h"
#include <string>
#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/logger.h>
#endif

IMPKERNEL_BEGIN_NAMESPACE

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
extern IMPKERNELEXPORT Flag<LogLevel, IMP_HAS_LOG != IMP_SILENT> log_level;
}
#endif

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
#ifndef IMP_DOXYGEN
#if !IMP_KERNEL_HAS_LOG4CXX
//! Push a new log context onto the stack
/** A log context is, eg, a function name.
 */
IMPKERNELEXPORT void push_log_context(const char *functionname,
                                    const void *object);

//! pop the top log context
IMPKERNELEXPORT void pop_log_context();

//! Write a string to the log
IMPKERNELEXPORT void add_to_log(std::string to_write);
#endif
#endif

//! Write a string to the log, for Python
IMPKERNELEXPORT void add_to_log(LogLevel level, std::string to_write);

//! Set the current global log level
/** @note may be overridden by set_log_level of specific objects that inherit
          from IMP::Object
    @note this global method should not, currently, be used directly during
    Model::evaluate() calls.

    \see get_log_level()
    \see IMP::Object::set_log_level()
*/
IMPKERNELEXPORT void set_log_level(LogLevel l);

//! Set whether log messages are tagged with the current log time
IMPKERNELEXPORT void set_log_timer(bool tb);

//! Reset the log timer
IMPKERNELEXPORT void reset_log_timer();

//! Get the currently active global log level
/** @note This may not always match the value passed to set_log_level()
          as objects can temporarily override the global level
          while they are evaluating.

    \see set_log_level()
    \see IMP::Object::set_log_level()

 */
inline LogLevel get_log_level() { return LogLevel(internal::log_level); }

#if IMP_KERNEL_HAS_LOG4CXX
inline log4cxx::LoggerPtr get_logger() {
  static log4cxx::LoggerPtr ret = log4cxx::Logger::getLogger("IMP");
  return ret;
}
#else
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline bool get_is_log_output(LogLevel l) { return l <= get_log_level(); }
#endif
#endif

/** @} */

/** \name Create a progress bar in the terminal

    @{
 */
//! Set up the progress bar with the passed description.
/** \see IMP_PROGRESS_DISPLAY().
*/
IMPKERNELEXPORT void set_progress_display(std::string description,
                                        unsigned int steps);

//! Set the current progress.
/** When it equals the number of steps, the bar is done. */
IMPKERNELEXPORT void add_to_progress_display(unsigned int step = 1);

/** @} */

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_LOG_H */
