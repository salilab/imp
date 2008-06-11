/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/exception.h"

namespace IMP
{

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

namespace internal
{

// The error message is already in the exception
bool print_exceptions=false;

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

} // namespace internal

} // namespace IMP
