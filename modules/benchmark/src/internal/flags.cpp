/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/internal/flags.h>
#include <IMP/base/flags.h>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

base::AdvancedFlag<int> run_only("run_only",
                                 "Run only one of the benchmarks, by index.",
                                 -1);
#if IMP_BASE_HAS_GPERFTOOLS
base::AdvancedFlag<bool> cpu_profile_benchmarks("cpu_profile_benchmarks",
                                                "Profile only the benchmarks",
                                                false);
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
bbase::AdvancedFlag<bool> heap_profile_benchmarks(
    "heap_profile_benchmarks", "Heap profile only the benchmarks", false);
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPCHECKER
base::AdvancedFlag<bool> heap_check_benchmarks("heap_check_benchmarks",
                                               "Heap check only the benchmarks",
                                               false);
#endif

IMPBENCHMARK_END_INTERNAL_NAMESPACE
