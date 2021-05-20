/**
 *  \file IMP/benchmark/flags.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_FLAGS_H
#define IMPBENCHMARK_FLAGS_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/Flag.h>
#include <boost/cstdint.hpp>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

extern IMPBENCHMARKEXPORT AdvancedFlag<int> run_only;

#if IMP_KERNEL_HAS_GPERFTOOLS
extern IMPBENCHMARKEXPORT AdvancedFlag<bool> cpu_profile_benchmarks;
#endif

#if IMP_KERNEL_HAS_TCMALLOC_HEAPPROFILER
extern IMPBENCHMARKEXPORT AdvancedFlag<bool> heap_profile_benchmarks;
#endif

#if IMP_KERNEL_HAS_TCMALLOC_HEAPCHECKER
extern IMPBENCHMARKEXPORT AdvancedFlag<bool> heap_check_benchmarks;
#endif

IMPBENCHMARK_END_INTERNAL_NAMESPACE

#endif /* IMPBENCHMARK_FLAGS_H */
