/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/enums.h"
#include "IMP/base/exception.h"
#include "IMP/base/check_macros.h"
#include <iostream>
IMPBASE_BEGIN_NAMESPACE
std::istream &operator>>(std::istream &in, LogLevel &ll) {
  std::string str;
  in >> str;
  if (str == "SILENT")
    ll = SILENT;
  else if (str == "WARNING")
    ll = WARNING;
  else if (str == "PROGRESS")
    ll = PROGRESS;
  else if (str == "TERSE")
    ll = TERSE;
  else if (str == "VERBOSE")
    ll = VERBOSE;
  else if (str == "MEMORY")
    ll = MEMORY;
  else {
    IMP_THROW("Bad log level " << str, IOException);
  }
  return in;
}

std::ostream &operator<<(std::ostream &in, LogLevel ll) {
  if (ll == SILENT)
    in << "SILENT";
  else if (ll == WARNING)
    in << "WARNING";
  else if (ll == PROGRESS)
    in << "PROGRESS";
  else if (ll == TERSE)
    in << "TERSE";
  else if (ll == VERBOSE)
    in << "VERBOSE";
  else if (ll == MEMORY)
    in << "MEMORY";
  else {
    IMP_THROW("Bad log level " << ll, IOException);
  }
  return in;
}

std::istream &operator>>(std::istream &in, CheckLevel &ll) {
  std::string str;
  in >> str;
  if (str == "NONE")
    ll = NONE;
  else if (str == "USAGE")
    ll = USAGE;
  else if (str == "USAGE_AND_INTERNAL")
    ll = USAGE_AND_INTERNAL;
  else {
    IMP_THROW("Bad log check " << str, IOException);
  }
  return in;
}

std::ostream &operator<<(std::ostream &in, CheckLevel ll) {
  if (ll== NONE) in << "NONE";
  else if (ll == USAGE) in << "USAGE";
  else if (ll == USAGE_AND_INTERNAL) in << "USAGE_AND_INTERNAL";
  else {
    IMP_THROW("Bad log check " << ll, IOException);
  }
  return in;
}

std::istream &operator>>(std::istream &in, StatisticsLevel &ll) {
  std::string str;
  in >> str;
  if (str == "NONE")
    ll = NO_STATISTICS;
  else if (str == "ALL")
    ll = ALL_STATISTICS;
  else {
    IMP_THROW("Bad statistics level " << str, IOException);
  }
  return in;
}

std::ostream &operator<<(std::ostream &out, StatisticsLevel ll) {
  if (ll == NO_STATISTICS)
    out << "NONE";
  else if (ll == ALL_STATISTICS)
    out << "ALL";
  else {
    IMP_THROW("Bad statistics level " << ll, IOException);
  }
  return out;
}

IMPBASE_END_NAMESPACE
