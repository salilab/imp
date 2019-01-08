/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/internal/flags.h>
#include <IMP/flags.h>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

AdvancedFlag<int> run_only("run_only",
                                 "Run only one of the benchmarks, by index.",
                                 -1);
#if IMP_KERNEL_HAS_GPERFTOOLS
AdvancedFlag<bool> cpu_profile_benchmarks("cpu_profile_benchmarks",
                                                "Profile only the benchmarks",
                                                false);
#endif
#if IMP_KERNEL_HAS_TCMALLOC_HEAPPROFILER
AdvancedFlag<bool> heap_profile_benchmarks(
    "heap_profile_benchmarks", "Heap profile only the benchmarks", false);
#endif
#if IMP_KERNEL_HAS_TCMALLOC_HEAPCHECKER
AdvancedFlag<bool> heap_check_benchmarks("heap_check_benchmarks",
                                               "Heap check only the benchmarks",
                                               false);
#endif

IMPBENCHMARK_END_INTERNAL_NAMESPACE
