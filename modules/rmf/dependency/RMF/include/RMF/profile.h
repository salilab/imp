/**
 *  \file RMF/profile.h
 *  \brief Various utilities for rmfing
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_PROFILE_H
#define RMF_PROFILE_H

#include <RMF/config.h>
#include <string>

RMF_COMPILER_ENABLE_WARNINGS
namespace RMF {

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
RMFEXPORT void set_profile_name(std::string name);
/** Turn on or off profiling.*/
RMFEXPORT void set_is_profiling(bool tf);
/** @} */

}

RMF_COMPILER_DISABLE_WARNINGS

#endif  /* RMF_PROFILE_H */
