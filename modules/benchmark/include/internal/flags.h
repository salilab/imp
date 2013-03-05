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
#include <boost/cstdint.hpp>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

extern IMPBENCHMARKEXPORT boost::int64_t run_only;

#if IMP_BASE_HAS_GPERFTOOLS
extern IMPBENCHMARKEXPORT bool cpu_profile_benchmarks;
#endif

#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
extern IMPBENCHMARKEXPORT bool heap_profile_benchmarks;
#endif

#if IMP_BASE_HAS_TCMALLOC_HEAPCHECKER
extern IMPBENCHMARKEXPORT bool heap_check_benchmarks;
#endif

IMPBENCHMARK_END_INTERNAL_NAMESPACE


#endif  /* IMPBENCHMARK_FLAGS_H */
