/** \file utility.cpp Benchmarking utilities
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/Profiler.h>
#include <IMP/kernel_config.h>
#if IMP_KERNEL_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif

#include <boost/format.hpp>
#include <IMP/log_macros.h>
#include <IMP/check_macros.h>
#include <IMP/utility.h>

IMPBENCHMARK_BEGIN_NAMESPACE

#if IMP_KERNEL_HAS_GPERFTOOLS
void Profiler::start(std::string name) {
  ProfilerStart(IMP::get_unique_name(name).c_str());
}
void Profiler::stop() { ProfilerStop(); }

#else
void Profiler::start(std::string) {
  // mostly useful in fast mode
  std::cerr << "GProfTools were not found, no profiling available."
            << std::endl;
}
void Profiler::stop() {}
#endif

IMPBENCHMARK_END_NAMESPACE
