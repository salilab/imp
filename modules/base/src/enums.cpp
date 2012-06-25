/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/enums.h"
#include "IMP/base/exception.h"
#include <iostream>
IMPBASE_BEGIN_NAMESPACE
std::istream &operator>>(std::istream &in,
                         LogLevel &ll) {
  std::string str;
  in >> str;
  if (str=="SILENT") ll=SILENT;
  else if (str=="WARNING") ll=WARNING;
  else if (str=="PROGRESS") ll=PROGRESS;
  else if (str=="TERSE") ll=TERSE;
  else if (str=="VERBOSE") ll=VERBOSE;
  else if (str=="MEMORY") ll=MEMORY;
  else {
    IMP_THROW("Bad log level " << str,
              IOException);
  }
  return in;
}


IMPBASE_END_NAMESPACE
