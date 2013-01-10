/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/threads.h"
#include "IMP/base/check_macros.h"
IMPBASE_BEGIN_NAMESPACE

namespace {
  unsigned int number_of_threads
#ifdef _OPENMP
  =3;
#else
=1;
#endif
}
unsigned int get_number_of_threads() {
  return number_of_threads;
}
void set_number_of_threads(unsigned int n) {
  IMP_USAGE_CHECK(n>0, "Can't have 0 threads.");
  number_of_threads=n;
}
IMPBASE_END_NAMESPACE
