/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/flags.h>

IMPBENCHMARK_BEGIN_NAMESPACE

IMP_DEFINE_INT(run_only, -1, "Run only one of the benchmarks, by index.");
#if defined(IMP_BASE_USE_GPERFTOOLS)
IMP_DEFINE_BOOL(cpu_profile_benchmarks, false, "Profile only the benchmarks");
#endif
#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPPROFILER)
IMP_DEFINE_BOOL(heap_profile_benchmarks, false,
                "Heap profile only the benchmarks");
#endif
#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPCHECKER)
IMP_DEFINE_BOOL(heap_check_benchmarks, false,
                "Heap check only the benchmarks");
#endif

IMPBENCHMARK_END_NAMESPACE
