/**
 *  \file benchmark/profile_macros.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_PROFILE_MACROS_H
#define IMPBENCHMARK_PROFILE_MACROS_H

#include "benchmark_config.h"
#include "profile.h"

#ifdef IMP_BENCHMARK_USE_GOOGLE_PERFTOOLS_PROFILE
#define IMP_BENCHMARK_START_PROFILING IMP::benchmark::set_is_profiling(true);
#define IMP_BENCHMARK_END_PROFILING IMP::benchmark::set_is_profiling(false);
#else
#define IMP_BENCHMARK_START_PROFILING
#define IMP_BENCHMARK_END_PROFILING

#endif

#endif  /* IMPBENCHMARK_PROFILE_MACROS_H */
