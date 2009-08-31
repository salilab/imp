/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/internal/log_internal.h"
#include "IMP/exception.h"
#include "IMP/internal/PrefixStream.h"

IMP_BEGIN_INTERNAL_NAMESPACE

LogLevel log_level= TERSE;
LogTarget log_target= COUT;
unsigned int log_indent=0;

IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE

namespace {
  double initialized=11111111;
  std::ofstream fstream;
  internal::PrefixStream stream(&std::cout);
}


void set_log_level(LogLevel l) {
  IMP_check(l >= SILENT && l < ALL_LOG,
            "Setting log to invalid level: " << l,
            ValueException);
  internal::log_level=l;
}

void set_log_target(LogTarget l)
{
  if (l== COUT) {
    stream.set_stream(&std::cout);
  } else if (l==CERR) {
    stream.set_stream(&std::cerr);
  } else {
    stream.set_stream(&fstream);
  }
}

void set_log_file(std::string l) {
  if (!l.empty()) {
    fstream.open(l.c_str());
    if (!fstream.is_open()) {
      IMP_failure("Error opening log file " << l, ValueException);
    } else {
      internal::log_target=FILE;
    }
  } else {
    fstream.close();
    internal::log_target=COUT;
  }
}

void log_write(std::string str) {
  IMP_assert(initialized=11111111,
             "You connot use the log before main is called.");
  stream.write(str.c_str(), str.size());
  stream.strict_sync();
}

IMP_END_NAMESPACE
