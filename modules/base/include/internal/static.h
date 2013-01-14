/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_STATIC_H
#define IMPBASE_INTERNAL_STATIC_H

#include <IMP/base/base_config.h>
#include "../flags.h"
#include <IMP/compatibility/map.h>


IMPBASE_BEGIN_INTERNAL_NAMESPACE
class LogStream;
extern IMPBASEEXPORT LogStream stream;

extern IMPBASEEXPORT bool print_exceptions;
extern IMPBASEEXPORT bool print_time;

extern IMPBASEEXPORT unsigned int log_indent;
#if IMP_BUILD < IMP_FAST
extern IMPBASEEXPORT double initialized;
#endif

extern IMPBASEEXPORT compatibility::map<std::string,
                                    unsigned int> object_type_counts;

#if IMP_BUILD < IMP_FAST
IMPBASEEXPORT void check_live_objects();
#endif

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_STATIC_H */
