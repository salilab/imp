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
#include "profile.h"
#include "profile_macros.h"
#include "command_line.h"
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
       (&IMP::benchmark::help)->zero_tokens(),                           \
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
    if (IMP::benchmark::profile_all) {                                  \
      IMP_BENCHMARK_START_PROFILING;                                    \
    }                                                                   \
  }

#else
#define IMP_BENCHMARK(extra_arguments)\
  IMP_UNUSED(argc);                   \
  IMP_UNUSED(argv)

#endif

#endif  /* IMPBENCHMARK_COMMAND_LINE_MACROS_H */
