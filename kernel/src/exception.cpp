/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/exception.h"
#include "IMP/FailureHandler.h"
#include "IMP/log.h"
#include "IMP/Object.h"
#include "IMP/internal/static.h"
#include "IMP/VectorOfRefCounted.h"
#include <cstring>
#include <boost/lambda/lambda.hpp>


IMP_BEGIN_NAMESPACE


CheckLevel get_maximum_check_level() {
#if IMP_BUILD == IMP_FAST
  return NONE;
#else
  return USAGE_AND_INTERNAL;
#endif
}

namespace internal {
void assert_fail(const char *msg)
{
  static bool is_handling=false;

  if (is_handling) {
    return;
  }
  is_handling=true;
  for (int i=internal::handlers.size()-1; i >=0; --i) {
    IMP_CHECK_OBJECT(internal::handlers[i]);
    try {
      handlers[i]->handle_failure();
    } catch (const Exception &e) {
      IMP_WARN("Caught exception in failure handler \""
               << internal::handlers[i]->get_name() << "\": "
               << e.what() << std::endl);
    }
  }
  if (print_exceptions) IMP_ERROR(msg);
  is_handling=false;
}
}

void add_failure_handler(FailureHandler *fh) {
  internal::handlers.push_back(fh);
  fh->set_was_used(true);
}


void remove_failure_handler(FailureHandler *fh) {
  internal::remove_if(internal::handlers, boost::lambda::_1 == fh);
}

ExceptionBase::~ExceptionBase() throw()
{
  destroy();
}

ExceptionBase::ExceptionBase(const char *message) {
  str_= new (std::nothrow) refstring();
  if (str_ != NULL) {
    str_->ct_=1;
    std::strncpy(str_->message_, message, 4095);
    str_->message_[4095]='\0';
  }
}



Exception::~Exception() throw()
{
}

Exception::Exception(const char *message):
  ExceptionBase(message) {
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

IOException::~IOException() throw()
{
}

void set_print_exceptions(bool ft) {
  internal::print_exceptions=ft;
}


IMP_END_NAMESPACE
