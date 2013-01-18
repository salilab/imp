/**
 *  \file IMP/base/threads.h
 *  \brief Control for using multiple threads in IMP
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_THREADS_H
#define IMPBASE_THREADS_H
#include "raii_macros.h"
#include <IMP/base/base_config.h>

IMPBASE_BEGIN_NAMESPACE

/** \name Number of threads
    Get and set the default number of threads to use in \imp.
    @{ */
/** Get the current number of threads requested. */
IMPBASEEXPORT unsigned int get_number_of_threads();
/** Set the current number of threads to a number greater or
    equal to 1. Setting it to 1 disables multithreaded evaluation.
*/
IMPBASEEXPORT void set_number_of_threads(unsigned int n);
/** @} */


/** Temporarily set the number of threads
 */
class SetNumberOfThreads: public base::RAII
{
  int num_;
public:
  IMP_RAII(SetNumberOfThreads, (unsigned int n),
           {num_= 0;},
           {
             num_=get_number_of_threads();
             set_number_of_threads(n);
           },
           {
             set_number_of_threads(num_);
             num_=0;
           }, );
};

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_THREADS_H */
