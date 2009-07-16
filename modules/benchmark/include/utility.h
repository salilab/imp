/**
 *  \file benchmark/utility.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_UTILITY_H
#define IMPBENCHMARK_UTILITY_H

#include "config.h"
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

//! Report a benchmark result in a standard way
/** The last value is a check value which can be used to make
    sure the computations were semi-ok. */
IMPBENCHMARKEXPORT void report(std::string name, double value,
                               double check);

IMPBENCHMARK_END_NAMESPACE

#endif  /* IMPBENCHMARK_UTILITY_H */
