/**
 *  \file IMP/benchmark/Profiler.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_PROFILER_H
#define IMPBENCHMARK_PROFILER_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/raii_macros.h>
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

/** One can use the
    [GPerfTools profiler](https://github.com/gperftools/gperftools)
    to profile code. On a Mac, you should
    use the Instruments program instead (part of the Mac OS X Developer Tools).
*/
class IMPBENCHMARKEXPORT Profiler : public RAII {
  void start(std::string name);
  void stop();

 public:
  IMP_RAII(Profiler, (std::string name), , start(name), stop(),
           out << "profiling");
};

IMPBENCHMARK_END_NAMESPACE

#endif /* IMPBENCHMARK_PROFILER_H */
