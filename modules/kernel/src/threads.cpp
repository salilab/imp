/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/threads.h"
#include "IMP/check_macros.h"
#include "IMP/flags.h"
#include "internal/base_static.h"
IMPKERNEL_BEGIN_NAMESPACE
unsigned int get_number_of_threads() { return internal::number_of_threads; }
void set_number_of_threads(unsigned int n) {
  IMP_USAGE_CHECK(n > 0, "Can't have 0 threads.");
#ifdef _OPENMP
  internal::number_of_threads = n;
#else
  IMP_UNUSED(n);
#endif
}
IMPKERNEL_END_NAMESPACE
