/**
 *  \file benchmark/HeapProfiler.h
 *  \brief A shared benchmark class to help in debugging and things.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_HEAP_PROFILER_H
#define IMPBENCHMARK_HEAP_PROFILER_H

#include "benchmark_config.h"
#include <IMP/base/raii_macros.h>


IMPBENCHMARK_BEGIN_NAMESPACE

/** Heap profiling monitors the memory that is used on the heap and where
    it came from. To use it, the
    \extern{http://gperftools.googlecode.com/,GPerTools} must be installed.
    */
class IMPBENCHMARKEXPORT HeapProfiler: public base::RAII {
  void start(std::string name);
  void stop();
 public:
  IMP_RAII(HeapProfiler, (std::string name),,start(name),
           stop(), out << "heap profiling");
  //! dump the current state of memory
  void dump();

};
IMPBENCHMARK_END_NAMESPACE

#endif  /* IMPBENCHMARK_HEAP_PROFILER_H */
