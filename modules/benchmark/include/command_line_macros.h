/**
 *  \file IMP/benchmark/command_line_macros.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_COMMAND_LINE_MACROS_H
#define IMPBENCHMARK_COMMAND_LINE_MACROS_H

#include <IMP/benchmark/benchmark_config.h>
#include "command_line.h"
#include "Profiler.h"
#include "internal/utility.h"
#include <IMP/base/utility.h>
#include <IMP/base/raii_macros.h>
#include <IMP/base/log_macros.h>
#include <boost/scoped_ptr.hpp>
#ifdef IMP_BENCHMARK_USE_BOOST_PROGRAMOPTIONS
#include <boost/program_options.hpp>
#endif

#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPPROFILER)
#include <gperftools/heap-profiler.h>
#endif
#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPCHECKER)
#include <gperftools/heap-checker.h>
#endif

// put it in header so that the symbols aren't brought into python
// as python and tcmalloc don't get along (due to the way python loads
// dynamic libraries with private symbols.
// the dummy template parameter is to prevent it from getting instantiated
// except in executables
#ifndef IMP_DOXYGEN
namespace {
/** Heap profiling monitors the memory that is used on the heap and where
    it came from. To use it, the
    \extern{http://gperftools.googlecode.com/,GPerTools} must be installed.
    */
template <int dummy>
class HeapProfiler: public IMP::base::RAII {
  std::string name_;
  void start(std::string name);
  void stop();
 public:
  IMP_RAII(HeapProfiler, (std::string name),,start(name),
           stop(), out << "heap profiling");
  //! dump the current state of memory to the file
  void dump(std::string name);

};

/** Leaking checking checks if things that were allocated on the stack were not
    freed. To use it, the
    \extern{http://gperftools.googlecode.com/,GPerTools} must be installed.

    The HEAPCHECK environment variable is
    used to control things.
*/
template <int dummy>
class LeakChecker: public IMP::base::RAII {
#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPCHECKER)
  boost::scoped_ptr<HeapLeakChecker> checker_;
#endif
  void start(std::string name);
  void stop();
 public:
  IMP_RAII(LeakChecker, (std::string name),,start(name),
           stop(), out << "leak checker");


};
#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPPROFILER)
template <int dummy>
void HeapProfiler<dummy>::start(std::string name) {
  name_=IMP::base::get_unique_name(name);
  HeapProfilerStart(name_.c_str());
}
template <int dummy>
void HeapProfiler<dummy>::stop(){
  dump(name_);
  HeapProfilerStop();
}
template <int dummy>
void HeapProfiler<dummy>::dump(std::string name){
  HeapProfilerDump(name.c_str());
}

#else // profiling support

template <int dummy>
void HeapProfiler<dummy>::start(std::string) {
  std::cerr << "GProfTools were not found, no profiling available."
            << std::endl;
}
template <int dummy>
void HeapProfiler<dummy>::stop(){}
template <int dummy>
void HeapProfiler<dummy>::dump(std::string name){
  IMP_UNUSED(name);
}
#endif


#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPCHECKER)
template <int dummy>
void LeakChecker<dummy>::start(std::string name) {
  std::string nname=IMP::base::get_unique_name(name);
  checker_.reset(new HeapLeakChecker(nname.c_str()));
}
template <int dummy>
void LeakChecker<dummy>::stop(){
  if (!checker_->NoLeaks()) {
    IMP_WARN("Memory leak detected" << std::endl);
  }
  checker_.reset();
}

#else

template <int dummy>
void LeakChecker<dummy>::start(std::string) {
  std::cerr << "GProfTools were not found, no profiling available."
            << std::endl;
}
template <int dummy>
void LeakChecker<dummy>::stop(){}

#endif
} // namespace

#endif


#ifdef IMP_BENCHMARK_USE_BOOST_PROGRAMOPTIONS
#define IMP_BENCHMARK(extra_arguments)                                  \
    boost::program_options::options_description desc;                   \
    desc.add_options()                                                  \
    ("help",                                                            \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::help)->zero_tokens(),                            \
     "Print help on command line arguments.");                          \
    desc.add_options()                                                  \
    ("profile",                                                         \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::profile_benchmark)->zero_tokens(),               \
     "Profile the benchmark itself.");                                  \
    desc.add_options()                                                  \
    ("log_level",                                                       \
     boost::program_options::value<IMP::base::LogLevel>                 \
     (&IMP::benchmark::log_level),                                      \
     "The logging level to use (if not in fast mode).");                \
    desc.add_options()                                                  \
    ("check_level",                                                     \
     boost::program_options::value<IMP::base::CheckLevel>               \
     (&IMP::benchmark::check_level),                                    \
     "The check level to use (if not in fast mode).");                  \
    desc.add_options()                                                  \
    ("profile_all",                                                     \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::profile_all)->zero_tokens(),                     \
     "Profile the whole program.");                                     \
    desc.add_options()                                                  \
    ("heap_profile",                                                    \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::heap_profile_benchmark)->zero_tokens(),          \
     "Profile the the heap usage for each benchmark.");                 \
    desc.add_options()                                                  \
    ("heap_profile_all",                                                \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::heap_profile_all)->zero_tokens(),                \
     "Profile the the heap usage.");                                    \
    desc.add_options()                                                  \
    ("leak_check",                                                      \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::leak_check_benchmark)->zero_tokens(),            \
     "Check for leaks in each benchmark.");                             \
    desc.add_options()                                                  \
    ("leak_check_all",                                                  \
     boost::program_options::value<bool>                                \
     (&IMP::benchmark::leak_check_all)->zero_tokens(),                  \
     "Check for leaks in the benchmark.");                              \
    desc.add_options()                                                  \
    ("run_only",                                                        \
     boost::program_options::value<int>                                 \
     (&IMP::benchmark::run_only),                                       \
     "Run only the ith benchmark.");                                    \
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
    IMP::base::set_log_level(IMP::benchmark::log_level);                \
    IMP::base::set_check_level(IMP::benchmark::check_level);            \
    IMP::benchmark::benchmarks_name                                     \
      = IMP::benchmark::internal::get_benchmarks_name(argv[0]);         \
    boost::scoped_ptr<IMP::benchmark::Profiler> profiler;               \
    if (IMP::benchmark::profile_all) {                                  \
      std::string name=IMP::benchmark::benchmarks_name+".all.pprof";    \
      profiler.reset(new IMP::benchmark::Profiler(name));               \
    }                                                                   \
    boost::scoped_ptr<HeapProfiler<0> > heap_profiler;                  \
    if (IMP::benchmark::heap_profile_all) {                             \
      std::string name=IMP::benchmark::benchmarks_name;                 \
      heap_profiler.reset(new HeapProfiler<0>(name));                   \
    }                                                                   \
    boost::scoped_ptr<LeakChecker<0> > leak_checker;                    \
    if (IMP::benchmark::leak_check_all) {                               \
      std::string name=IMP::benchmark::benchmarks_name+".all";          \
      leak_checker.reset(new LeakChecker<0>(name));                     \
    }

#else
#define IMP_BENCHMARK(extra_arguments)\
  IMP_UNUSED(argc);                   \
  IMP_UNUSED(argv)

#endif

#endif  /* IMPBENCHMARK_COMMAND_LINE_MACROS_H */
