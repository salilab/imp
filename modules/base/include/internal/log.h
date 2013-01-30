/**
 *  \file internal/log.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_LOG_H
#define IMPBASE_INTERNAL_LOG_H

#include <IMP/base/base_config.h>
#include "../enums.h"
#if IMP_BASE_HAS_LOG4CXX
#  include <log4cxx/logger.h>
#  include <log4cxx/helpers/messagebuffer.h>
#endif

IMPBASE_BEGIN_INTERNAL_NAMESPACE

#if IMP_BASE_HAS_LOG4CXX
namespace log {
  // eat std::endl and std::flush
  typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
  typedef CoutType& (*StandardEndLine)(CoutType&);
  inline log4cxx::helpers::CharMessageBuffer&
  operator<<(log4cxx::helpers::CharMessageBuffer& buf,
             StandardEndLine) {
    buf << "\n";
    return buf;
  }
  inline log4cxx::helpers::MessageBuffer&
  operator<<(log4cxx::helpers::MessageBuffer& buf,
             StandardEndLine) {
    buf << "\n";
    return buf;
  }
}

IMPBASEEXPORT void init_logger();
#else
IMPBASEEXPORT extern unsigned int log_indent;
IMPBASEEXPORT extern bool print_time;
#endif

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_LOG_H */
