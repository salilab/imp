/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/internal/utility.h>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

std::string get_benchmarks_name(const char* name) {
  std::string sname(name);
  std::size_t loc = sname.rfind("benchmark_");
  if (loc != std::string::npos) {
    return std::string(name + loc + 10);
  } else {
    int sloc = sname.rfind("/");
    return std::string(name + sloc);
  }
}

IMPBENCHMARK_END_INTERNAL_NAMESPACE
