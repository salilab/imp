/**
 *  \file IMP/benchmark/Profiler.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_PROFILER_H
#define IMPBENCHMARK_PROFILER_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/base/raii_macros.h>
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

/** One can use the
    \external{http://code.google.com/p/gperftools/
    the GPerfTools profiler} to profile code. On a mac, you should
    use the Instruments program instead (part of the mac os developer tools).
*/
class IMPBENCHMARKEXPORT Profiler : public base::RAII {
  void start(std::string name);
  void stop();

 public:
  IMP_RAII(Profiler, (std::string name), , start(name), stop(),
           out << "profiling");

};

IMPBENCHMARK_END_NAMESPACE

#endif /* IMPBENCHMARK_PROFILER_H */
