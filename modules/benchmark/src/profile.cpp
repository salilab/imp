/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/profile.h>
#ifdef IMP_BENCHMARK_USE_GOOGLE_PERFTOOLS_PROFILE
#include <google/profiler.h>
#endif

IMPBENCHMARK_BEGIN_NAMESPACE

#ifdef IMP_BENCHMARK_USE_GOOGLE_PERFTOOLS_PROFILE
namespace {
  std::string profname="imp.prof";
}

void set_profile_name(std::string name) {
  profname=name;
}

void set_is_profiling(bool tf) {
  if (tf) {
    ProfilerStart(profname.c_str());
  } else {
    ProfilerStop();
  }
}
#endif

IMPBENCHMARK_END_NAMESPACE
