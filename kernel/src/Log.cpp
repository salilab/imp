/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/internal/log_internal.h"
#include "IMP/exception.h"

IMP_BEGIN_INTERNAL_NAMESPACE

/* Initialize singleton pointer to NULL */
Log* Log::logpt_ = NULL;

Log &Log::get() {
  if (!logpt_) {
    logpt_ = new Log(TERSE, COUT);
  }
  return *logpt_;
}


IMP_END_INTERNAL_NAMESPACE
