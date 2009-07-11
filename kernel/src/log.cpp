/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/internal/log_internal.h"
#include "IMP/exception.h"

IMP_BEGIN_INTERNAL_NAMESPACE

Log &Log::get() {
  static Log log(COUT);
  return log;
}


LogLevel log_level= TERSE;

IMP_END_INTERNAL_NAMESPACE
