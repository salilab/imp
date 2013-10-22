/**
 *  \file IMP/benchmark/utility.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_UTILITY_H
#define IMPBENCHMARK_UTILITY_H

#include <IMP/benchmark/benchmark_config.h>
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

//! Report a benchmark result in a standard way.
/** \param[in] benchmark the name of the benchmark being run
    \param[in] algorithm the name of the particular algorithm
    used to solve the problem
    \param[in] time the time it took (perhaps normalized)
    \param[in] check a check value to print out to see if the calc was ok
*/
IMPBENCHMARKEXPORT void report(std::string benchmark, std::string algorithm,
                               double time, double check);

#ifndef IMP_DOXYGEN
IMPBENCHMARKEXPORT void report(std::string benchmark, double time,
                               double check);
#endif

IMPBENCHMARKEXPORT int get_return_value();

IMPBENCHMARK_END_NAMESPACE

#endif /* IMPBENCHMARK_UTILITY_H */
