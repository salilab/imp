/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_STATIC_H
#define IMP_INTERNAL_STATIC_H

#include "../kernel_config.h"
#include "log_stream.h"
#include "../exception.h"
#include "../FailureHandler.h"
#include "map.h"




IMP_BEGIN_INTERNAL_NAMESPACE

extern IMPEXPORT internal::Map<std::string, unsigned int> object_type_counts;

extern IMPEXPORT bool print_exceptions;

extern IMPEXPORT VectorOfRefCounted<FailureHandler*> handlers;

extern IMPEXPORT CheckLevel check_mode;

extern IMPEXPORT LogLevel log_level;
extern IMPEXPORT unsigned int log_indent;
IMP_CHECK_CODE(extern IMPEXPORT double initialized);
extern IMPEXPORT internal::LogStream stream;

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_STATIC_H */
