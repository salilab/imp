/**
 *  \file internal/log.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LOG_H
#define IMPKERNEL_INTERNAL_LOG_H

#include <IMP/kernel_config.h>
#include "../enums.h"
#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/helpers/messagebuffer.h>
#endif

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

#if IMP_KERNEL_HAS_LOG4CXX
namespace log {
// eat std::endl and std::flush
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
typedef CoutType& (*StandardEndLine)(CoutType&);
inline log4cxx::helpers::CharMessageBuffer& operator<<(
    log4cxx::helpers::CharMessageBuffer& buf, StandardEndLine) {
  buf << "\n";
  return buf;
}
inline log4cxx::helpers::MessageBuffer& operator<<(
    log4cxx::helpers::MessageBuffer& buf, StandardEndLine) {
  buf << "\n";
  return buf;
}
}

IMPKERNELEXPORT void init_logger();
#else
IMPKERNELEXPORT extern unsigned int log_indent;
IMPKERNELEXPORT extern bool print_time;
#endif

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_LOG_H */
