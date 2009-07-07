/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/exception.h"
#include "IMP/log.h"

IMP_BEGIN_NAMESPACE

namespace {
FailureFunction failure_function=NULL;
}

namespace internal {

CheckLevel check_mode =
#ifdef NDEBUG
  NONE;
#else
  EXPENSIVE;
#endif

}

void set_failure_function(FailureFunction f) {
  failure_function=f;
}


Exception::~Exception() throw()
{
  destroy();
}

ErrorException::~ErrorException() throw()
{
}

InvalidStateException::~InvalidStateException() throw()
{
}

InactiveParticleException::~InactiveParticleException() throw()
{
}

IndexException::~IndexException() throw()
{
}

ValueException::~ValueException() throw()
{
}

IOException::~IOException() throw()
{
}

namespace
{

// The error message is already in the exception
bool print_exceptions=true;


}

void set_print_exceptions(bool ft) {
  print_exceptions=ft;
}


void assert_fail(const char *msg)
{
  if (failure_function && !failure_function(msg)) {
  } else {
    if (print_exceptions) {
      IMP_ERROR(msg);
    }
    throw ErrorException(msg);
  }
}

bool check_fail(const char *msg)
{
  if (failure_function && !failure_function(msg)) {
    return false;
  } else {
    if (print_exceptions) {
      IMP_ERROR(msg);
    }
    return true;
  }
}

IMP_END_NAMESPACE
