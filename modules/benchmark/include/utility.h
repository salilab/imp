/**
 *  \file benchmark/utility.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_UTILITY_H
#define IMPBENCHMARK_UTILITY_H

#include "benchmark_config.h"
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

//! Report a benchmark result in a standard way
/**  */
IMPBENCHMARKEXPORT void report(std::string name, double value,
                               double check);

IMPBENCHMARKEXPORT std::pair<double,double> get_baseline();

IMPBENCHMARKEXPORT int get_return_value();

IMPBENCHMARK_END_NAMESPACE

#endif  /* IMPBENCHMARK_UTILITY_H */
