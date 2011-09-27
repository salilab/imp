/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_STATIC_H
#define IMPBASE_INTERNAL_STATIC_H

#include "../base_config.h"
#include "../log.h"
#include "../exception.h"
#include <IMP/compatibility/map.h>


IMPBASE_BEGIN_INTERNAL_NAMESPACE
class LogStream;

extern IMPBASEEXPORT bool print_exceptions;

extern IMPBASEEXPORT CheckLevel check_mode;

extern IMPBASEEXPORT LogLevel log_level;
extern IMPBASEEXPORT unsigned int log_indent;
IMP_CHECK_CODE(extern IMPBASEEXPORT double initialized);
extern IMPBASEEXPORT LogStream stream;

extern IMPBASEEXPORT compatibility::map<std::string,
                                    unsigned int> object_type_counts;

extern IMPBASEEXPORT FailureHandlers handlers;

#if IMP_BUILD < IMP_FAST
IMPBASEEXPORT void check_live_objects();
#endif

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_STATIC_H */
