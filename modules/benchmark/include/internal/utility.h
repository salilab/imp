/**
 *  \file benchmark/utility.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_INTERNAL_UTILITY_H
#define IMPBENCHMARK_INTERNAL_UTILITY_H

#include <IMP/benchmark/benchmark_config.h>
#include <string>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

//! get the name to use for this set of benchmarks from argv[0]
IMPBENCHMARKEXPORT std::string get_benchmarks_name(const char* name);

IMPBENCHMARK_END_INTERNAL_NAMESPACE

#endif /* IMPBENCHMARK_INTERNAL_UTILITY_H */
