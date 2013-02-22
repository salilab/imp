/**
 *  \file RMF/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 RMF Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_COMPILER_MACROS_H
#define RMF_INTERNAL_COMPILER_MACROS_H

#ifdef __clang__
#define RMF_COMPILER_HAS_OVERRIDE 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define RMF_COMPILER_HAS_OVERRIDE 1
#else
#define RMF_COMPILER_HAS_OVERRIDE 0
#endif

#if RMF_COMPILER_HAS_OVERRIDE && !defined(SWIG)
#define RMF_OVERRIDE override
#else
#define RMF_OVERRIDE
#endif


#ifdef __clang__
#define RMF_COMPILER_HAS_FINAL 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define RMF_COMPILER_HAS_FINAL 1
#else
#define RMF_COMPILER_HAS_FINAL 0
#endif

#if RMF_COMPILER_HAS_FINAL && !defined(SWIG)
#define RMF_FINAL final
#else
#define RMF_FINAL
#endif

#ifdef __clang__
#define RMF_COMPILER_HAS_PRAGMA 1
#elif defined(__GNUC__)
// probably should be finer here
#define RMF_COMPILER_HAS_PRAGMA 1
#else
#define RMF_COMPILER_HAS_PRAGMA 0
#endif

#if RMF_COMPILER_HAS_PRAGMA
#define RMF_PRAGMA(x) _Pragma(RMF_STRINGIFY(x))
#elif defined(_MSC_VER)
#define RMF_PRAGMA(x) __pragma(x)
#else
#define RMF_PRAGMA(x)
#endif


#endif  /* RMF_INTERNAL_COMPILER_MACROS_H */
