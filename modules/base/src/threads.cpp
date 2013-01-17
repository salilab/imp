/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/threads.h"
#include "IMP/base/check_macros.h"
#include "IMP/base/flags.h"
IMPBASE_BEGIN_NAMESPACE
#ifdef _OPENMP
IMP_DEFINE_INT(number_of_threads, 2,
               "The number of threads to use within IMP.");
#else
static const int FLAGS_number_of_threads=1;
#endif

unsigned int get_number_of_threads() {
  return FLAGS_number_of_threads;
}
void set_number_of_threads(unsigned int n) {
  IMP_USAGE_CHECK(n>0, "Can't have 0 threads.");
#ifdef _OPENMP
  FLAGS_number_of_threads=n;
#endif
}
IMPBASE_END_NAMESPACE
