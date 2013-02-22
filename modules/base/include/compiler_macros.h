/**
 *  \file IMP/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_BASE_COMPILER_MACROS_H
#define IMPBASE_BASE_COMPILER_MACROS_H

#define IMP_STRINGIFY(x) #x

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

#ifdef __clang__
#define IMP_COMPILER_HAS_OVERRIDE 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define IMP_COMPILER_HAS_OVERRIDE 1
#else
#define IMP_COMPILER_HAS_OVERRIDE 0
#endif

#if IMP_COMPILER_HAS_OVERRIDE && !defined(SWIG)
#define IMP_OVERRIDE override
#else
#define IMP_OVERRIDE
#endif


#ifdef __clang__
#define IMP_COMPILER_HAS_FINAL 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define IMP_COMPILER_HAS_FINAL 1
#else
#define IMP_COMPILER_HAS_FINAL 0
#endif

#if IMP_COMPILER_HAS_FINAL && !defined(SWIG)
#define IMP_FINAL final
#else
#define IMP_FINAL
#endif

#ifdef __clang__
#define IMP_COMPILER_HAS_PRAGMA 1
#elif defined(__GNUC__)
// probably should be finer here
#define IMP_COMPILER_HAS_PRAGMA 1
#else
#define IMP_COMPILER_HAS_PRAGMA 0
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

#ifdef __clang__
#define IMP_COMPILER_ON_BEGIN_NAMESPACE \
 IMP_PRAGMA(clang diagnostic push)\
 IMP_PRAGMA(clang diagnostic warning "-Wall")
 IMP_PRAGMA(clang diagnostic ignored "-Wunknown-pragmas")\
 IMP_PRAGMA(clang diagnostic ignored "-Wpadded")\
 IMP_PRAGMA(clang diagnostic ignored "-Wc++11-extensions")\
 IMP_PRAGMA(clang diagnostic ignored "-Wc++98-compat")

#define IMP_COMPILER_ON_END_NAMESPACE \
IMP_PRAGMA(clang diagnostic pop)

#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
             "-Wstrict-aliasing=2",
             -fno-operator-names",]*/
#if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#define IMP_GCC_PUSH_POP(x) IMP_PRAGMA(x)
#define IMP_GCC_CXX0X_COMPAT\
 IMP_PRAGMA(GCC diagnostic ignored "-Wc++0x-compat")
#define IMP_GCC_PROTOTYPES \
IMP_PRAGMA(GCC diagnostic warning "-Wmissing-declarations")
#else
#define IMP_GCC_PUSH_POP(x)
#define IMP_GCC_CXX0X_COMPAT
#define IMP_GCC_PROTOTYPES
#endif

#define IMP_COMPILER_ON_BEGIN_NAMESPACE \
IMP_GCC_PUSH_POP(GCC diagnostic push) \
IMP_PRAGMA(GCC diagnostic warning "-Wall") \
IMP_PRAGMA(GCC diagnostic warning "-Wextra") \
IMP_PRAGMA(GCC diagnostic warning "-Winit-self") \
IMP_PRAGMA(GCC diagnostic warning "-Wcast-align") \
IMP_PRAGMA(GCC diagnostic warning "-Woverloaded-virtual") \
IMP_PRAGMA(GCC diagnostic ignored "-Wunknown-pragmas") \
IMP_PRAGMA(GCC diagnostic warning "-Wundef") \
IMP_GCC_PROTOTYPES \
IMP_GCC_CXX0X_COMPAT

#define IMP_COMPILER_ON_END_NAMESPACE \
IMP_GCC_PUSH_POP(GCC diagnostic pop)

#else
#define IMP_COMPILER_ON_BEGIN_NAMESPACE
#define IMP_COMPILER_ON_END_NAMESPACE
#endif

#endif  /* IMPBASE_BASE_COMPILER_MACROS_H */
