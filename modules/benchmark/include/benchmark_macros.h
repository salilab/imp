/**
 *  \file IMP/benchmark/benchmark_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_MACROS_H
#define IMPBENCHMARK_MACROS_H

#include "command_line.h"
#include "Profiler.h"
#include "command_line_macros.h"
#include <boost/timer.hpp>
#include <boost/scoped_ptr.hpp>
#include <IMP/base/exception.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#define IMP_BENCHMARK_RUN                                               \
  if (IMP::benchmark::run_only <0                                       \
      || (IMP::benchmark::run_only>=0                                   \
          && IMP::benchmark::run_only                                   \
          == IMP::benchmark::next_benchmark++))

#define IMP_BENCHMARK_PROFILING                                         \
  boost::scoped_ptr<IMP::benchmark::Profiler> profiler;                 \
    if (IMP::benchmark::profile_benchmark) {                            \
      std::string name=IMP::benchmark::benchmarks_name+".%1%.pprof";    \
      profiler.reset(new IMP::benchmark::Profiler(name));               \
    }                                                                   \
    boost::scoped_ptr<HeapProfiler<0> > heap_profiler;                  \
    if (IMP::benchmark::heap_profile_benchmark) {                       \
      std::string name=IMP::benchmark::benchmarks_name+".%1%";          \
      heap_profiler.reset(new HeapProfiler<0>(name));                   \
    }                                                                   \
    boost::scoped_ptr<LeakChecker<0> > leak_checker;                    \
    if (IMP::benchmark::leak_check_benchmark) {                         \
      std::string name=IMP::benchmark::benchmarks_name+".%1%.leaks";    \
      leak_checker.reset(new LeakChecker<0>(name));                     \
    }


//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. See also IMP_TIME_N */
#define IMP_TIME(block, timev)                                          \
  IMP_BENCHMARK_RUN {                                                   \
    boost::timer imp_timer;                                             \
    unsigned int imp_reps=0;                                            \
    IMP_BENCHMARK_PROFILING;                                            \
    try {                                                               \
      do {                                                              \
        block;                                                          \
        ++imp_reps;                                                     \
      } while (imp_timer.elapsed() < 2.5);                              \
    } catch (const IMP::base::Exception &e) {                           \
      std::cerr<< "Caught exception "                                   \
               << e.what() << std::endl;                                \
    }                                                                   \
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
    IMP_BENCHMARK_PROFILING;                                            \
    try {                                                               \
      do {                                                              \
        block;                                                          \
        ++imp_reps;                                                     \
      } while (microsec_clock::local_time()-start < seconds(2.5));      \
    } catch (const IMP::base::Exception &e) {                           \
      std::cerr<< "Caught exception "                                   \
               << e.what() << std::endl;                                \
    }                                                                   \
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
    IMP_BENCHMARK_PROFILING;                                            \
    for (unsigned int i=0; i< (N); ++i) {                               \
      try {                                                             \
        block;                                                          \
      } catch (const IMP::base::Exception &e) {                         \
        std::cerr<< "Caught exception "                                 \
                 << e.what() << std::endl;                              \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    timev= imp_timer.elapsed()/(N);                                     \
  } else {                                                              \
    timev=-1;                                                           \
  }


#endif  /* IMPBENCHMARK_MACROS_H */
