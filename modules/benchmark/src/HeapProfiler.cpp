/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/benchmark/HeapProfiler.h>
#include <IMP/base/log_macros.h>
#if defined(IMP_BENCHMARK_USE_GPROFTOOLS)
#include <gperftools/heap-profiler.h>
#elif defined(IMP_BENCHMARK_USE_GOOGLEPROFTOOLS)
#include <google/heap-profiler.h>
#endif


IMPBENCHMARK_BEGIN_NAMESPACE
#if defined(IMP_BENCHMARK_USE_GPROFTOOLS) \
  || defined(IMP_BENCHMARK_USE_GOOGLEPROFTOOLS)

void HeapProfiler::start(std::string name) {
  HeapProfilerStart(base::get_unique_name(name).c_str());
}
void HeapProfiler::stop(){
  HeapProfilerStop();
}
void HeapProfiler::dump(){
  HeapProfilerDump();
}

#else
void HeapProfiler::start(std::string) {
  IMP_WARN("GProfTools were not found, no profiling available.\n");
}
void HeapProfiler::stop(){}

#endif

IMPBENCHMARK_END_NAMESPACE
