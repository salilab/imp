/**
 *  \file benchmark/profile.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_PROFILE_H
#define IMPBENCHMARK_PROFILE_H

#include "benchmark_config.h"
#include <string>

IMPBENCHMARK_BEGIN_NAMESPACE

/** \name Profiling
    One can use the
    \external{http://http://code.google.com/p/google-perftools/,
    google perftools profiler} to profile code. On a mac, you should
    use the Instruments program instead (part of the mac os developer tools).
    @{
*/
/** Set the name of the file to use for profiling results. If the name contains
    a %1%, that will be replaced by the index of the time profiling it turned on
    (for when you are profiling multiple bits of a single run).

    By default, the name is something like "imp.0.prof".
*/
IMPBENCHMARKEXPORT void set_profile_name(std::string name);
/** Turn on or off profiling.*/
IMPBENCHMARKEXPORT void set_is_profiling(bool tf);
/** @} */

IMPBENCHMARK_END_NAMESPACE

#endif  /* IMPBENCHMARK_PROFILE_H */
