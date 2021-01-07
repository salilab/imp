/**
 *  \file RMF/log.h
 *  \brief Functions and macros for logging.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_LOG_H
#define RMF_LOG_H

#include <string>

#include "RMF/config.h"
#if RMF_HAS_LOG4CXX
#include <log4cxx/logger.h>                 // IWYU pragma: export
#include <log4cxx/ndc.h>                    // IWYU pragma: export
#include <log4cxx/helpers/messagebuffer.h>  // IWYU pragma: export
#include <log4cxx/helpers/objectptr.h>      // IWYU pragma: export
#include <log4cxx/logger.h>                 // IWYU pragma: export
#endif

RMF_ENABLE_WARNINGS

namespace RMF {

#if RMF_HAS_LOG4CXX
RMFEXPORT log4cxx::LoggerPtr get_logger();
#endif

/** Set the log level from a string. Supported values are:
    - Trace
    - Info
    - Warn
    - Error
    - Off

   Log4CXX is required for logging to be supported.
*/
RMFEXPORT void set_log_level(std::string level);

} /* namespace RMF */

#if RMF_HAS_LOG4CXX

#define RMF_TRACE(expr) LOG4CXX_TRACE(RMF::get_logger(), expr)
#define RMF_DEBUG(expr) LOG4CXX_DEBUG(RMF::get_logger(), expr)
#define RMF_INFO(expr) LOG4CXX_INFO(RMF::get_logger(), expr)
#define RMF_WARN(expr) LOG4CXX_WARN(RMF::get_logger(), expr)
#define RMF_ERROR(expr) LOG4CXX_ERROR(RMF::get_logger(), expr)
#define RMF_FATAL(expr) LOG4CXX_FATAL(RMF::get_logger(), expr)
#define RMF_CONTEXT(expr) log4cxx::NDC _log_context(expr)

#else
#define RMF_TRACE(expr)
#define RMF_DEBUG(expr)
#define RMF_INFO(expr)
#define RMF_WARN(expr)
#define RMF_ERROR(expr)
#define RMF_FATAL(expr)
#define RMF_CONTEXT(expr)

#endif

RMF_DISABLE_WARNINGS

#endif /* RMF_LOG_H */
