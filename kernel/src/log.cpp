/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/exception.h"
#include "IMP/file.h"
#include "IMP/internal/static.h"



IMP_BEGIN_INTERNAL_NAMESPACE

IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE

void set_log_level(LogLevel l) {
  IMP_USAGE_CHECK(l >= SILENT && l < ALL_LOG,
            "Setting log to invalid level: " << l);
  internal::log_level=l;
}

void set_log_target(TextOutput l)
{
  internal::stream.set_stream(l);
}

TextOutput get_log_target()
{
  return internal::stream.get_stream();
}


void add_to_log(std::string str) {
  IMP_INTERNAL_CHECK(static_cast<int>(internal::initialized)==11111111,
                     "You connot use the log before main is called.");
  internal::stream.write(str.c_str(), str.size());
  internal::stream.strict_sync();
}

IMP_END_NAMESPACE
