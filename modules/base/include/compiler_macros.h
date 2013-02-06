/**
 *  \file IMP/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_BASE_COMPILER_MACROS_H
#define IMPBASE_BASE_COMPILER_MACROS_H

#include <IMP/base/base_config.h>
// recommended by http://gcc.gnu.org/gcc/Function-Names.html
#if defined(_MSC_VER)
#  define __func__ __FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif


#ifndef IMP_DOXYGEN
#ifdef __GNUC__
//! Use this to label a function with no side effects
/** \advanced */
#define IMP_NO_SIDEEFFECTS __attribute__ ((pure))
//! Use this to make the compiler (possibly) warn if the result is not used
/** \advanced */
#define IMP_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
//! restrict means that a variable is not aliased with this function
#define IMP_RESTRICT __restrict__

#else
#define IMP_NO_SIDEEFFECTS
#define IMP_WARN_UNUSED_RESULT
#define IMP_RESTRICT
#endif

#endif

#if IMP_COMPILER_HAS_OVERRIDE && !defined(SWIG)
#define IMP_OVERRIDE override
#else
#define IMP_OVERRIDE
#endif

#if IMP_COMPILER_HAS_FINAL && !defined(SWIG)
#define IMP_FINAL final
#else
#define IMP_FINAL
#endif


#if IMP_COMPILER_HAS_PRAGMA
#define IMP_PRAGMA(x) _Pragma(IMP_STRINGIFY(x))
#elif defined(_MSC_VER)
#define IMP_PRAGMA(x) __pragma(x)
#else
#define IMP_PRAGMA(x)
#endif

#if IMP_COMPILER_LITTLE_ENDIAN
#define IMP_LITTLE_ENDIAN
#else
#define IMP_BIG_ENDIAN
#endif

#define IMP_DEBUG 0
#define IMP_RELEASE 1
#define IMP_FAST 2


#endif  /* IMPBASE_BASE_COMPILER_MACROS_H */
