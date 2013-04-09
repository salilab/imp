/**
 *  \file IMP/benchmark/benchmark_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_MACROS_H
#define IMPBENCHMARK_MACROS_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/base/flags.h>
#include <boost/timer.hpp>
#include <boost/scoped_ptr.hpp>
#include "internal/control.h"
#include "internal/flags.h"
#include <IMP/base/exception.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#if IMP_BASE_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
#include <gperftools/heap-profiler.h>
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPCHECKER
#include <gperftools/heap-checker.h>
#endif

#define IMP_BENCHMARK_RUN                                               \
  ++IMP::benchmark::internal::current_benchmark;                        \
  if ((IMP::benchmark::internal::run_only <0                            \
       || (IMP::benchmark::internal::run_only>=0                        \
           && IMP::benchmark::internal::run_only                        \
           == IMP::benchmark::internal::current_benchmark)))

#if IMP_BASE_HAS_GPERFTOOLS
#define IMP_BENCHMARK_CPU_PROFILING_BEGIN                               \
  if (IMP::benchmark::internal::cpu_profile_benchmarks) {               \
    ProfilerStart(IMP::benchmark::internal::get_file_name(".pprof").c_str()); \
  }
#define IMP_BENCHMARK_CPU_PROFILING_END                   \
  if (IMP::benchmark::internal::cpu_profile_benchmarks) { \
    ProfilerStop();                                       \
  }
#else
#define IMP_BENCHMARK_CPU_PROFILING_BEGIN
#define IMP_BENCHMARK_CPU_PROFILING_END
#endif

#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
#define IMP_BENCHMARK_HEAP_PROFILING_BEGIN                              \
  if (IMP::benchmark::internal::heap_profile_benchmarks) {              \
    HeapProfilerStart(IMP::benchmark::internal                          \
                      ::get_file_name(".hprof").c_str());               \
  }
#define IMP_BENCHMARK_HEAP_PROFILING_END                        \
  if (IMP::benchmark::internal::heap_profile_benchmarks) {      \
    HeapProfilerStop();                                         \
  }
#else
#define IMP_BENCHMARK_HEAP_PROFILING_BEGIN
#define IMP_BENCHMARK_HEAP_PROFILING_END
#endif

#if IMP_BASE_HAS_TCMALLOC_HEAPCHECKER
#define IMP_BENCHMARK_HEAP_CHECKING_BEGIN                               \
  boost::scoped_ptr<HeapLeakChecker> heap_checker;                      \
  if (IMP::benchmark::internal::heap_check_benchmarks) {                \
  heap_checker.reset(new HeapLeakChecker(IMP::benchmark                 \
                                    ::internal::get_file_name("").c_str())); \
  }
#define IMP_BENCHMARK_HEAP_CHECKING_END                         \
  if (IMP::benchmark::internal::heap_check_benchmarks) {        \
    if (!heap_checker->NoLeaks()) std::cerr << "Leaks found\n";  \
    heap_checker.reset(nullptr);                                 \
  }
#else
#define IMP_BENCHMARK_HEAP_CHECKING_BEGIN
#define IMP_BENCHMARK_HEAP_CHECKING_END
#endif

#define IMP_BENCHMARK_PROFILING_BEGIN           \
  IMP_BENCHMARK_CPU_PROFILING_BEGIN             \
  IMP_BENCHMARK_HEAP_CHECKING_BEGIN             \
  IMP_BENCHMARK_HEAP_PROFILING_BEGIN

#define IMP_BENCHMARK_PROFILING_END           \
  IMP_BENCHMARK_CPU_PROFILING_END             \
  IMP_BENCHMARK_HEAP_CHECKING_END             \
  IMP_BENCHMARK_HEAP_PROFILING_END


//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. See also IMP_TIME_N */
#define IMP_TIME(block, timev)                                          \
  IMP_BENCHMARK_RUN {                                                   \
    boost::timer imp_timer;                                             \
    unsigned int imp_reps=0;                                            \
    IMP_BENCHMARK_PROFILING_BEGIN;                                      \
    try {                                                               \
      do {                                                              \
        block;                                                          \
        ++imp_reps;                                                     \
      } while (imp_timer.elapsed() < 2.5 && !IMP::base::run_quick_test); \
    } catch (const IMP::base::Exception &e) {                           \
      std::cerr<< "Caught exception "                                   \
               << e.what() << std::endl;                                \
    }                                                                   \
    IMP_BENCHMARK_PROFILING_END;                                        \
    timev= imp_timer.elapsed()/imp_reps;                                \
  } else {                                                              \
    timev=-1;                                                           \
  }

//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. See also IMP_TIME_N */
#define IMP_WALLTIME(block, timev)                                      \
  IMP_BENCHMARK_RUN {                                                   \
    using namespace boost::posix_time;                                  \
    ptime start=microsec_clock::local_time();                           \
    unsigned int imp_reps=0;                                            \
    IMP_BENCHMARK_PROFILING_BEGIN;                                      \
    try {                                                               \
      do {                                                              \
        block;                                                          \
        ++imp_reps;                                                     \
      } while (microsec_clock::local_time()-start < seconds(2)          \
               && !IMP::base::run_quick_test);                          \
    } catch (const IMP::base::Exception &e) {                           \
      std::cerr<< "Caught exception "                                   \
               << e.what() << std::endl;                                \
    }                                                                   \
    IMP_BENCHMARK_PROFILING_END;                                        \
    timev= (microsec_clock::local_time()-start)                         \
      .total_milliseconds()/1000.0                                      \
      /imp_reps;                                                        \
  } else {                                                              \
    timev=-1;                                                           \
  }

//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. The bit of code is run
    exact N times. See also IMP_TIME */
#define IMP_TIME_N(block, timev, N)                                     \
  IMP_BENCHMARK_RUN {                                                   \
    boost::timer imp_timer;                                             \
    IMP_BENCHMARK_PROFILING_BEGIN;                                      \
    for (unsigned int i=0; i< (N); ++i) {                               \
      try {                                                             \
        block;                                                          \
      } catch (const IMP::base::Exception &e) {                         \
        std::cerr<< "Caught exception "                                 \
                 << e.what() << std::endl;                              \
        break;                                                          \
      }                                                                 \
      if (IMP::base::run_quick_test) break;                             \
    }                                                                   \
    IMP_BENCHMARK_PROFILING_END;                                        \
    timev= imp_timer.elapsed()/(N);                                     \
  } else {                                                              \
    timev=-1;                                                           \
  }


#endif  /* IMPBENCHMARK_MACROS_H */
