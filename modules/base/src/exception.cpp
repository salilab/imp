/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/exception.h"
#include "IMP/base/log.h"
#include "IMP/base/internal/static.h"
#include "IMP/base/check_macros.h"
#include <cstring>
#include <boost/lambda/lambda.hpp>


IMPBASE_BEGIN_NAMESPACE

CheckLevel get_maximum_check_level() {
#if IMP_BUILD == IMP_FAST
  return NONE;
#else
  return USAGE_AND_INTERNAL;
#endif
}

void handle_error(const char *msg)
{
  IMP_LOG_VARIABLE(msg);
  if (internal::print_exceptions) {
    IMP_ERROR(msg);
  }
}

Exception::~Exception() throw()
{
}

Exception::Exception(const char *message):
  std::runtime_error(message) {
}

InternalException::~InternalException() throw()
{
}

UsageException::~UsageException() throw()
{
}

IndexException::~IndexException() throw()
{
}

ValueException::~ValueException() throw()
{
}

ModelException::~ModelException() throw()
{
}

EventException::~EventException() throw()
{
}

IOException::~IOException() throw()
{
}

void set_print_exceptions(bool ft) {
  internal::print_exceptions=ft;
}


IMPBASE_END_NAMESPACE
