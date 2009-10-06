/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/exception.h"
#include "IMP/FailureHandler.h"
#include "IMP/log.h"
#include "IMP/VectorOfRefCounted.h"

#include <boost/lambda/lambda.hpp>

IMP_BEGIN_NAMESPACE

namespace {
  // The error message is already in the exception
  bool print_exceptions=true;

  VectorOfRefCounted<FailureHandler*> handlers;

}

namespace internal {
void assert_fail(const char *msg)
{
  for (int i=handlers.size()-1; i >=0; --i) {
    IMP_CHECK_OBJECT(handlers[i]);
    handlers[i]->handle_failure();
  }
  if (print_exceptions) IMP_ERROR(msg);
}
}

void add_failure_handler(FailureHandler *fh) {
  handlers.push_back(fh);
}


void remove_failure_handler(FailureHandler *fh) {
  handlers.remove_if(boost::lambda::_1 == fh);
}


namespace internal {
 CheckLevel check_mode =
#ifdef IMP_NDEBUG
    NONE;
#else
  EXPENSIVE;
#endif
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

void set_print_exceptions(bool ft) {
  print_exceptions=ft;
}


IMP_END_NAMESPACE
