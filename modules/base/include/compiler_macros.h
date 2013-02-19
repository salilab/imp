/**
 *  \file IMP/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_BASE_COMPILER_MACROS_H
#define IMPBASE_BASE_COMPILER_MACROS_H

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

#ifdef __clang__
#define IMP_COMPILER_ON_BEGIN_NAMESPACE \
 _Pragma("clang diagnostic push")\
 _Pragma("clang diagnostic warn \"-Wall\"")
 _Pragma("clang diagnostic ignored \"-Wunknown-pragmas\"")\
 _Pragma("clang diagnostic ignored \"-Wpadded\"")\
 _Pragma("clang diagnostic ignored \"-Wc++11-extensions\"")

#define IMP_COMPILER_ON_END_NAMESPACE \
_Pragma("clang diagnostic pop")

#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
             "-Wstrict-aliasing=2",
             -fno-operator-names",]
        if float(options.version) >= 4.2:
            if sys.platform == 'darwin':
                ret+=["-Wmissing-prototypes"]
            else:
                ret+=["-Wmissing-declarations"]
        if float(options.version) >= 4.6:
            ret+=["-Wno-c++0x-compat"]
        #if dependency.gcc.get_version(env)>= 4.3:
        #    env.Append(CXXFLAGS=["-Wunsafe-loop-optimizations"])
        # gcc 4.0 on Mac doesn't like -isystem, so we don't use it there.
        # But without -isystem, -Wundef throws up lots of Boost warnings.
        if sys.platform != 'darwin' or float(options.version) > 4.0:
            ret+=["-Wundef"]*/
#if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#define IMP_GCC_CXX0X_COMPAT\
 _Pragma("GCC diagnostic ignored \"-Wc++0x-compat\"")
#else
define IMP_GCC_CXX0X_COMPAT
#endif

#define IMP_COMPILER_ON_BEGIN_NAMESPACE \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic warning \"-Wall\"") \
_Pragma("GCC diagnostic warning \"-Wextra\"") \
_Pragma("GCC diagnostic warning \"-Winit-self\"") \
_Pragma("GCC diagnostic warning \"-Wcast-align\"") \
_Pragma("GCC diagnostic warning \"-Woverloaded-virtual\"") \
_Pragma("GCC diagnostic ignored \"-Wunknown-pragmas\"") \
_Pragma("GCC diagnostic warning \"-Wundef\"") \
IMP_GCC_CXX0X_COMPAT

#define IMP_COMPILER_ON_END_NAMESPACE \
_Pragma("GCC diagnostic pop")

#else
#define IMP_COMPILER_ON_BEGIN_NAMESPACE
#define IMP_COMPILER_ON_END_NAMESPACE
#endif

#endif  /* IMPBASE_BASE_COMPILER_MACROS_H */
