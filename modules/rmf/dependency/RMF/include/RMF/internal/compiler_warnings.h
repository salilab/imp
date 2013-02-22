/**
 *  \file RMF/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 RMF Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_COMPILER_WARNINGS_H
#define RMF_INTERNAL_COMPILER_WARNINGS_H

#ifdef __clang__
#pragma clang diagnostic warning "-Wall"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wc++11-extensions"

#elif defined(__GNUC__)

#ifdef __APPLE__
#pragma GCC diagnostic warning "-Wmissing-prototypes"
#else
#pragma GCC diagnostic warning "-Wmissing-declarations"
#endif

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"
#pragma GCC diagnostic warning "-Winit-self"
#pragma GCC diagnostic warning "-Wcast-align"
#pragma GCC diagnostic warning "-Woverloaded-virtual"
#pragma GCC diagnostic warning "-Wundef"

#endif

#endif  /* RMF_INTERNAL_COMPILER_WARNINGS_H */
