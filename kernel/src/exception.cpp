/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/exception.h"

IMP_BEGIN_NAMESPACE

static CheckLevel check_mode =
#ifdef NDEBUG
  CHEAP;
#else
  EXPENSIVE;
#endif

void set_check_level(CheckLevel cm)
{
  check_mode = cm;
}

CheckLevel get_check_level()
{
  return check_mode;
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

namespace
{

// The error message is already in the exception
bool print_exceptions=false;


}

void set_print_exceptions(bool ft) {
  print_exceptions=ft;
}


void assert_fail(const char *msg)
{
  if (print_exceptions) {
    IMP_ERROR(msg);
  }
  throw ErrorException(msg);
}

void check_fail(const char *msg)
{
  if (print_exceptions) {
    IMP_ERROR(msg);
  }
}

IMP_END_NAMESPACE
