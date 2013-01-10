/**
 *  \file IMP/benchmark/flags.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_FLAGS_H
#define IMPBENCHMARK_FLAGS_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/base/flag_macros.h>

IMPBENCHMARK_BEGIN_NAMESPACE

IMP_DECLARE_INT(BENCHMARK, run_only);

#if defined(IMP_BASE_USE_GPERFTOOLS)
IMP_DECLARE_BOOL(BENCHMARK, cpu_profile_benchmarks);
#endif

#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPPROFILER)
IMP_DECLARE_BOOL(BENCHMARK, heap_profile_benchmarks);
#endif

#if defined(IMP_BENCHMARK_USE_TCMALLOC_HEAPCHECKER)
IMP_DECLARE_BOOL(BENCHMARK, heap_check_benchmarks);
#endif

IMPBENCHMARK_END_NAMESPACE


#endif  /* IMPBENCHMARK_FLAGS_H */
