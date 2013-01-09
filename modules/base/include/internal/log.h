/**
 *  \file internal/log.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_LOG_H
#define IMPBASE_INTERNAL_LOG_H

#include <IMP/base/base_config.h>
#include "../enums.h"


IMPBASE_BEGIN_INTERNAL_NAMESPACE

IMPBASEEXPORT extern LogLevel log_level;
IMPBASEEXPORT extern unsigned int log_indent;
IMPBASEEXPORT extern bool print_time;

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_LOG_H */
