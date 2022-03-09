/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/exception.h"
#include "IMP/warning_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
void handle_error(const char *message) {
  IMP_UNUSED(message);
  // this method is just here to provide a place to break in the debugger
}

Exception::~Exception() IMP_NOEXCEPT {}

Exception::Exception(const char *message) : std::runtime_error(message) {}

InternalException::~InternalException() IMP_NOEXCEPT {}

UsageException::~UsageException() IMP_NOEXCEPT {}

IndexException::~IndexException() IMP_NOEXCEPT {}

ValueException::~ValueException() IMP_NOEXCEPT {}

ModelException::~ModelException() IMP_NOEXCEPT {}

EventException::~EventException() IMP_NOEXCEPT {}

IOException::~IOException() IMP_NOEXCEPT {}

TypeException::~TypeException() IMP_NOEXCEPT {}

IMPKERNEL_END_NAMESPACE
