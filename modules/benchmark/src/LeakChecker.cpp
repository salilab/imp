/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/benchmark/LeakChecker.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/utility.h>
#if defined(IMP_BENCHMARK_USE_GPERFTOOLS)
#include <gperftools/heap-checker.h>
#elif defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)
#include <google/heap-checker.h>
#endif


IMPBENCHMARK_BEGIN_NAMESPACE
#if defined(IMP_BENCHMARK_USE_GPERFTOOLS) \
  || defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)

void LeakChecker::start(std::string name) {
  std::string nname=base::get_unique_name(name);
  checker_.reset(new HeapLeakChecker(nname.c_str()));
}
void LeakChecker::stop(){
  if (!checker_->NoLeaks()) {
    IMP_WARN("Memory leak detected" << std::endl);
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
