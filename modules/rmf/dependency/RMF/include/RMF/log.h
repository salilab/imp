/**
 *  \file RMF/Log.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_LOG_H
#define RMF_LOG_H

#include <RMF/config.h>
#include <string>
#if RMF_HAS_LOG4CXX
#  include <log4cxx/logger.h>
#  include <log4cxx/ndc.h>
#endif

RMF_ENABLE_WARNINGS
namespace RMF {

#if RMF_HAS_LOG4CXX
RMFEXPORT log4cxx::LoggerPtr get_logger();
#ifndef IMP_DOXYGEN
RMFEXPORT log4cxx::LoggerPtr get_avro_logger();
RMFEXPORT log4cxx::LoggerPtr get_hdf5_logger();
#endif
#endif

/** Set the log level from a string. Supported values are:
    - Trace
    - Info
    - Warn
    - Error
    - Off
*/
RMFEXPORT void set_log_level(std::string level);

} /* namespace RMF */

#if RMF_HAS_LOG4CXX

#  define RMF_TRACE(log, expr) LOG4CXX_TRACE(log, expr)
#  define RMF_DEBUG(log, expr) LOG4CXX_DEBUG(log, expr)
#  define RMF_INFO(log, expr) LOG4CXX_INFO(log, expr)
#  define RMF_WARN(log, expr) LOG4CXX_WARN(log, expr)
#  define RMF_ERROR(log, expr) LOG4CXX_ERROR(log, expr)
#  define RMF_FATAL(log, expr) LOG4CXX_FATAL(log, expr)
#  define RMF_CONTEXT(expr) log4cxx::NDC _log_context(expr)

#else
#  define RMF_TRACE(log, expr)
#  define RMF_DEBUG(log, expr)
#  define RMF_INFO(log, expr)
#  define RMF_WARN(log, expr)
#  define RMF_ERROR(log, expr)
#  define RMF_FATAL(log, expr)
#  define RMF_CONTEXT(expr)

#endif

RMF_DISABLE_WARNINGS

#endif /* RMF_LOG_H */
