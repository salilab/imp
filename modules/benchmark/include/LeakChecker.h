/**
 *  \file benchmark/LeakChecker.h
 *  \brief A shared benchmark class to help in debugging and things.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_LEAK_CHECKER_H
#define IMPBENCHMARK_LEAK_CHECKER_H

#include "benchmark_config.h"
#include <IMP/base/raii_macros.h>
#include <boost/scoped_ptr.hpp>

#if defined(IMP_BENCHMARK_USE_GPERFTOOLS)
#include <gperftools/heap-checker.h>
#elif defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)
#include <google/heap-checker.h>
#endif

IMPBENCHMARK_BEGIN_NAMESPACE

/** Leaking checking checks if things that were allocated on the stack were not
    freed. To use it, the
    \extern{http://gperftools.googlecode.com/,GPerTools} must be installed.

    The HEAPCHECK environment variable is
    used to control things.
*/
class IMPBENCHMARKEXPORT LeakChecker: public base::RAII {
#if defined(IMP_BENCHMARK_USE_GPERFTOOLS)\
  || defined(IMP_BENCHMARK_USE_GOOGLEPERFTOOLS)
  boost::scoped_ptr<HeapLeakChecker> checker_;
#endif
  void start(std::string name);
  void stop();
 public:
  IMP_RAII(LeakChecker, (std::string name),,start(name),
           stop(), out << "leak checker");

};
IMPBENCHMARK_END_NAMESPACE

#endif  /* IMPBENCHMARK_LEAK_CHECKER_H */
