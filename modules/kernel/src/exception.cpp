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

Exception::~Exception() noexcept {}

Exception::Exception(const char *message) : std::runtime_error(message) {}

InternalException::~InternalException() noexcept {}

UsageException::~UsageException() noexcept {}

IndexException::~IndexException() noexcept {}

ValueException::~ValueException() noexcept {}

ModelException::~ModelException() noexcept {}

EventException::~EventException() noexcept {}

IOException::~IOException() noexcept {}

TypeException::~TypeException() noexcept {}

IMPKERNEL_END_NAMESPACE
