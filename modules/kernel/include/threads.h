/**
 *  \file IMP/threads.h
 *  \brief Control for using multiple threads in IMP
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_THREADS_H
#define IMPKERNEL_THREADS_H
#include "raii_macros.h"
#include <IMP/kernel_config.h>

IMPKERNEL_BEGIN_NAMESPACE
/** \name Number of threads
    Get and set the default number of threads to use in
    \imp.
    @{ */
/** Get the current number of threads requested. The default
    is the number of cores/hardware threads in the machine if
    there is OpenMP support, or 1 otherwise. */
IMPKERNELEXPORT unsigned int get_number_of_threads();
/** Set the current number of threads to a number greater or
    equal to 1. Setting it to 1 disables multithreaded evaluation.
*/
IMPKERNELEXPORT void set_number_of_threads(unsigned int n);
/** @} */

/** Temporarily set the number of threads
 */
class SetNumberOfThreads : public RAII {
  int num_;

 public:
  IMP_RAII(SetNumberOfThreads, (unsigned int n), { num_ = 0; },
  {
    num_ = get_number_of_threads();
    set_number_of_threads(n);
  },
  {
    if (num_) set_number_of_threads(num_);
    num_ = 0;
  }, );
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_THREADS_H */
