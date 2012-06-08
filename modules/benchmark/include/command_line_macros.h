/**
 *  \file benchmark/command_line_macros.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_COMMAND_LINE_MACROS_H
#define IMPBENCHMARK_COMMAND_LINE_MACROS_H

#include "benchmark_config.h"
#include "command_line.h"
#include "Profiler.h"
#include "HeapProfiler.h"
#include "LeakChecker.h"
#include <boost/scoped_ptr.hpp>
#ifdef IMP_BENCHMARK_USE_BOOST_PROGRAMOPTIONS
#include <boost/program_options.hpp>
#endif

#ifdef IMP_BENCHMARK_USE_BOOST_PROGRAMOPTIONS
#define IMP_BENCHMARK(extra_arguments)                                  \
  {                                                                     \
    boost::program_options::options_description desc;                   \
    desc.add_options()                                                  \
      ("help",                                                          \
       boost::program_options::value<bool>                              \
       (&IMP::benchmark::help)->zero_tokens(),                          \
       "Print help on command line arguments.");                        \
    desc.add_options()                                                  \
      ("profile",                                                       \
       boost::program_options::value<bool>                              \
       (&IMP::benchmark::profile_benchmark)->zero_tokens(),             \
       "Profile the benchmark itself.");                                \
    desc.add_options()                                                  \
      ("profile_all",                                                   \
       boost::program_options::value<bool>                              \
       (&IMP::benchmark::profile_all)->zero_tokens(),                   \
       "Profile the whole program.");                                   \
    desc.add_options()                                                  \
        ("heap_profile",                                                \
         boost::program_options::value<bool>                            \
         (&IMP::benchmark::heap_profile_benchmark)->zero_tokens(),      \
         "Profile the the heap usage for each benchmark.");             \
    desc.add_options()                                                  \
        ("heap_profile_all",                                            \
         boost::program_options::value<bool>                            \
         (&IMP::benchmark::heap_profile_all)->zero_tokens(),            \
         "Profile the the heap usage.");                                \
    desc.add_options()                                                  \
        ("leak_check",                                                  \
       boost::program_options::value<bool>                              \
         (&IMP::benchmark::leak_check_benchmark)->zero_tokens(),        \
         "Check for leaks in each benchmark.");                         \
    desc.add_options()                                                  \
        ("leak_check_all",                                              \
       boost::program_options::value<bool>                              \
         (&IMP::benchmark::leak_check_all)->zero_tokens(),              \
         "Check for leaks in the benchmark.");                          \
    desc.add_options()                                                  \
      extra_arguments;                                                  \
    boost::program_options::variables_map vm;                           \
    boost::program_options::store(boost::program_options                \
                                  ::parse_command_line(argc,            \
                                                       argv,            \
                                                       desc),           \
                                  vm);                                  \
    boost::program_options::notify(vm);                                 \
    if (vm.count("help")) {                                             \
      std::cout << desc << "\n";                                        \
      return 1;                                                         \
    }                                                                   \
    boost::scoped_ptr<IMP::benchmark::Profiler> profiler;               \
    if (IMP::benchmark::profile_all) {                                  \
      profiler.reset(new IMP::benchmark::Profiler("benchmark.%1%.pprof")); \
    }                                                                   \
    boost::scoped_ptr<IMP::benchmark::HeapProfiler> heap_profiler;      \
    if (IMP::benchmark::heap_profile_all) {                             \
      heap_profiler.reset(new                                           \
                          IMP::benchmark::HeapProfiler("benchmark.hprof")); \
    }                                                                   \
    boost::scoped_ptr<IMP::benchmark::LeakChecker> leak_checker;        \
    if (IMP::benchmark::leak_check_all) {                               \
      leak_checker.reset(new                                            \
                         IMP::benchmark::LeakChecker("benchmark.%1%.leaks")); \
    }                                                                   \
  }

#else
#define IMP_BENCHMARK(extra_arguments)\
  IMP_UNUSED(argc);                   \
  IMP_UNUSED(argv)

#endif

#endif  /* IMPBENCHMARK_COMMAND_LINE_MACROS_H */
