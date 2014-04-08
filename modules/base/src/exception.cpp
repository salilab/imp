/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/exception.h"
#include "IMP/base/warning_macros.h"

IMPBASE_BEGIN_NAMESPACE
void handle_error(const char *message) {
  IMP_UNUSED(message);
  // this method is just here to provide a place to break in the debugger
}

Exception::~Exception() throw() {}

Exception::Exception(const char *message) : std::runtime_error(message) {}

InternalException::~InternalException() throw() {}

UsageException::~UsageException() throw() {}

IndexException::~IndexException() throw() {}

ValueException::~ValueException() throw() {}

ModelException::~ModelException() throw() {}

EventException::~EventException() throw() {}

IOException::~IOException() throw() {}

IMPBASE_END_NAMESPACE
