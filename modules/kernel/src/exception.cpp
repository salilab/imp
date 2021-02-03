/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/exception.h"
#include "IMP/warning_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
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

TypeException::~TypeException() throw() {}

IMPKERNEL_END_NAMESPACE
