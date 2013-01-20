/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/Profiler.h>
#include <IMP/base/base_config.h>
#if IMP_BASE_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif

#include <boost/format.hpp>
#include <IMP/base/log_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/utility.h>

IMPBENCHMARK_BEGIN_NAMESPACE

#if IMP_BASE_HAS_GPERFTOOLS
void Profiler::start(std::string name) {
  ProfilerStart(base::get_unique_name(name).c_str());
}
void Profiler::stop(){
  ProfilerStop();
}

#else
void Profiler::start(std::string) {
  // mostly useful in fast mode
  std::cerr << "GProfTools were not found, no profiling available."
            << std::endl;
}
void Profiler::stop(){}
#endif

IMPBENCHMARK_END_NAMESPACE
