/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/internal/flags.h>
#include <IMP/base/flags.h>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

boost::int64_t run_only=-1;
base::AddIntFlag rof("run_only",
                     "Run only one of the benchmarks, by index.",
                     &run_only);
#if IMP_BASE_HAS_GPERFTOOLS
bool cpu_profile_benchmarks=false;
base::AddBoolFlag cpbf("cpu_profile_benchmarks",
                       "Profile only the benchmarks",
                       &cpu_profile_benchmarks);
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
bool heap_profile_benchmarks=false;
base::AddBoolFlag hpbf("heap_profile_benchmarks",
                       "Heap profile only the benchmarks",
                       &heap_profile_benchmarks);
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPCHECKER
bool heap_check_benchmarks=false;
base::AddBoolFlag hcbf("heap_check_benchmarks",
                       "Heap check only the benchmarks",
                       &heap_check_benchmarks);
#endif

IMPBENCHMARK_END_INTERNAL_NAMESPACE
