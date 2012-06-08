/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/benchmark/LeakChecker.h>
#include <IMP/base/log_macros.h>
#if defined(IMP_BENCHMARK_USE_GPROFTOOLS)
#include <gperftools/heap-checker.h>
#elif defined(IMP_BENCHMARK_USE_GOOGLEPROFTOOLS)
#include <google/heap-checker.h>
#endif


IMPBENCHMARK_BEGIN_NAMESPACE
#if defined(IMP_BENCHMARK_USE_GPROFTOOLS) \
  || defined(IMP_BENCHMARK_USE_GOOGLEPROFTOOLS)


void LeakChecker::start(std::string name) {
  checker_.reset(new HeapLeakChecker(base::get_unique_name(name)));
}
void LeakChecker::stop(){
  if (!checker_->NoLeaks()) {
    // prevent loop
    checker_.reset();
    IMP_THROW("Memory leak detected", RuntimeError);
  }
  checker_.reset();
}

#else

void LeakChecker::start(std::string) {
  IMP_WARN("GProfTools were not found, no profiling available.\n");
}
void LeakChecker::stop(){}

#endif

IMPBENCHMARK_END_NAMESPACE
