/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/benchmark/HeapProfiler.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/utility.h>
#if defined(IMP_BENCHMARK_USE_GPERFTOOLS)
#include <gperftools/heap-profiler.h>
#elif defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)
#include <google/heap-profiler.h>
#endif


IMPBENCHMARK_BEGIN_NAMESPACE
#if defined(IMP_BENCHMARK_USE_GPERFTOOLS) \
  || defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)

void HeapProfiler::start(std::string name) {
  name_=base::get_unique_name(name);
  HeapProfilerStart(name_.c_str());
}
void HeapProfiler::stop(){
  dump(name_);
  HeapProfilerStop();
}
void HeapProfiler::dump(std::string name){
  HeapProfilerDump(name.c_str());
}

#else
void HeapProfiler::start(std::string) {
  std::cerr << "GProfTools were not found, no profiling available."
            << std::endl;
}
void HeapProfiler::stop(){}

void HeapProfiler::dump(){}

#endif

IMPBENCHMARK_END_NAMESPACE
