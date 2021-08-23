/**
 *  \file IMP/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_COMPILER_MACROS_H
#define IMPKERNEL_COMPILER_MACROS_H

#include <boost/config.hpp>
#include <boost/version.hpp>
#if defined(BOOST_NO_CXX11_RANGE_BASED_FOR)
#include <boost/foreach.hpp>
#define IMP_FOREACH(v, r) BOOST_FOREACH(v, r)
#else
/** Use C++11 range-based for if available or BOOST_FOREACH if not. */
#define IMP_FOREACH(v, r) for (v : r)
#endif

#define IMP_STRINGIFY(x) #x

// recommended by http://gcc.gnu.org/gcc/Function-Names.html
#if defined(_MSC_VER)
#define IMP_CURRENT_FUNCTION __FUNCTION__
#define IMP_CURRENT_PRETTY_FUNCTION __FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
#define IMP_CURRENT_FUNCTION __FUNCTION__
#define IMP_CURRENT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define IMP_CURRENT_FUNCTION __func__
#define IMP_CURRENT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#ifndef IMP_DOXYGEN
#ifdef __GNUC__
#define IMP_NO_SIDEEFFECTS __attribute__((pure))
#define IMP_UNUSED_FUNCTION __attribute__((unused))
#define IMP_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define IMP_RESTRICT __restrict__

#else
#define IMP_NO_SIDEEFFECTS
#define IMP_UNUSED_FUNCTION
#define IMP_WARN_UNUSED_RESULT
#define IMP_RESTRICT
#endif

#else
//! Use this to label a function with no side effects
/** \advancedmethod */
#define IMP_NO_SIDEEFFECTS
//! Use this to make the compiler (possibly) warn if the result is not used
/** \advancedmethod */
#define IMP_WARN_UNUSED_RESULT
//! Label a function that is never called
/** \advancedmethod */
#define IMP_UNUSED_FUNCTION
//! restrict means that a variable is not aliased with this function
#define IMP_RESTRICT
#endif

#if defined(__clang__) && __clang_major__ >= 5
#define IMP_COMPILER_HAS_OVERRIDE 1
#elif !defined(__clang__) && defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define IMP_COMPILER_HAS_OVERRIDE 1
#else
#define IMP_COMPILER_HAS_OVERRIDE 0
#endif

#ifdef IMP_DOXYGEN
//! Cause a compile error if this method does not override a parent method
/** This is helpful to catch accidental mismatches of call signatures between
    the method and the parent method, which would cause the method to be
    overloaded rather than overridden. Usually this macro should be used
    whenever implementing a method that is declared virtual in the parent. */
#define IMP_OVERRIDE
#else
#if IMP_COMPILER_HAS_OVERRIDE
#define IMP_OVERRIDE override
#else
#define IMP_OVERRIDE
#endif
#endif

#if defined(IMP_SWIG_WRAPPER)
#define IMP_COMPILER_HAS_FINAL 0
#elif defined(__clang__)
#define IMP_COMPILER_HAS_FINAL 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define IMP_COMPILER_HAS_FINAL 1
#else
#define IMP_COMPILER_HAS_FINAL 0
#endif

#ifdef IMP_DOXYGEN
//! Have the compiler report an error if anything overrides this method
#define IMP_FINAL
#else
#if IMP_COMPILER_HAS_FINAL
#define IMP_FINAL final
#else
#define IMP_FINAL
#endif
#endif

#if defined(__GNUC__) && __cplusplus >= 201103L
#define IMP_HAS_NOEXCEPT 1
#elif defined(__clang__) && defined(__has_feature)
#define IMP_HAS_NOEXCEPT __has_feature(cxx_noexcept)
#else
#define IMP_HAS_NOEXCEPT 0
#endif

#if IMP_HAS_NOEXCEPT
#define IMP_NOEXCEPT noexcept
#define IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(Name) \
  Name(const Name &) = default;                  \
  Name &operator=(const Name &) = default
#else
// probably should be finer here
#define IMP_NOEXCEPT throw()
#define IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(Name)
#endif

#if defined(__clang__) || defined(__GNUC__)
#define IMP_PRAGMA(x) _Pragma(IMP_STRINGIFY(x))

#if defined(__clang__)
#define IMP_CLANG_PRAGMA(x) IMP_PRAGMA(clang x)
#define IMP_GCC_PRAGMA(x)
#define IMP_VC_PRAGMA(x)
#else
#define IMP_CLANG_PRAGMA(x)
#define IMP_GCC_PRAGMA(x) IMP_PRAGMA(GCC x)
#define IMP_VC_PRAGMA(x)
#endif

#elif defined(_MSC_VER)
#define IMP_PRAGMA(x) __pragma(x)
#define IMP_CLANG_PRAGMA(x)
#define IMP_GCC_PRAGMA(x)
#define IMP_VC_PRAGMA(x) IMP_PRAGMA(x)

#else
#define IMP_PRAGMA(x)
#define IMP_CLANG_PRAGMA(x)
#define IMP_GCC_PRAGMA(x)
#define IMP_VC_PRAGMA(x)
#endif

#ifdef __clang__

#define IMP_GCC_PUSH_POP(x)

#define IMP_COMPILER_ENABLE_WARNINGS                            \
  IMP_CLANG_PRAGMA(diagnostic push)                             \
      IMP_CLANG_PRAGMA(diagnostic warning "-Wall")              \
      IMP_CLANG_PRAGMA(diagnostic warning "-Wextra")            \
      IMP_CLANG_PRAGMA(diagnostic warning "-Weverything")       \
      IMP_CLANG_PRAGMA(diagnostic ignored "-Wconversion")       \
      IMP_CLANG_PRAGMA(diagnostic ignored "-Wc++11-extensions") \
      IMP_CLANG_PRAGMA(diagnostic ignored "-Wc++11-compat")     \
      IMP_CLANG_PRAGMA(diagnostic warning "-Wsign-compare")     \
      IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-member-function")

#define IMP_HELPER_MACRO_PUSH_WARNINGS IMP_CLANG_PRAGMA(diagnostic push)

#define IMP_HELPER_MACRO_POP_WARNINGS IMP_CLANG_PRAGMA(diagnostic pop)

#define IMP_COMPILER_DISABLE_WARNINGS IMP_CLANG_PRAGMA(diagnostic pop)

#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
  "-Wstrict-aliasing=2",
  -fno-operator-names",]*/
#if __GNUC__ > 4 || __GNUC_MINOR__ >= 6
#define IMP_GCC_PUSH_POP(x) IMP_PRAGMA(x)
#define IMP_GCC_CXX0X_COMPAT        \
  IMP_GCC_PRAGMA(diagnostic ignored \
                 "-Wc++0x-compat")
#ifdef IMP_SWIG_WRAPPER
#define IMP_GCC_PROTOTYPES
#else
#define IMP_GCC_PROTOTYPES \
  IMP_GCC_PRAGMA(diagnostic warning "-Wmissing-declarations")
#endif

#define IMP_HELPER_MACRO_PUSH_WARNINGS IMP_GCC_PRAGMA(diagnostic push)

#define IMP_HELPER_MACRO_POP_WARNINGS IMP_GCC_PRAGMA(diagnostic pop)

#else
#define IMP_GCC_PUSH_POP(x)
#define IMP_GCC_CXX0X_COMPAT
#define IMP_GCC_PROTOTYPES
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

// Warn about missing IMP_OVERRIDE on virtual methods if gcc is new enough
#if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#ifdef IMP_SWIG_WRAPPER
#define IMP_GCC_OVERRIDE
#else
#define IMP_GCC_OVERRIDE \
  IMP_GCC_PRAGMA(diagnostic warning "-Wsuggest-override")
#endif
#else
#define IMP_GCC_OVERRIDE
#endif

#define IMP_COMPILER_ENABLE_WARNINGS                                 \
  IMP_GCC_PUSH_POP(GCC diagnostic push)                              \
      IMP_GCC_PRAGMA(diagnostic warning "-Wall")                     \
      IMP_GCC_PRAGMA(diagnostic warning "-Wextra")                   \
      IMP_GCC_PRAGMA(diagnostic warning "-Winit-self")               \
      IMP_GCC_PRAGMA(diagnostic warning "-Wcast-align")              \
      IMP_GCC_PRAGMA(diagnostic warning "-Woverloaded-virtual")      \
      IMP_GCC_PRAGMA(diagnostic warning "-Wdeprecated-declarations") \
      IMP_GCC_PRAGMA(diagnostic warning "-Wundef")                   \
      IMP_GCC_PROTOTYPES IMP_GCC_CXX0X_COMPAT IMP_GCC_OVERRIDE

#define IMP_COMPILER_DISABLE_WARNINGS IMP_GCC_PUSH_POP(GCC diagnostic pop)

#elif defined(_MSC_VER)
#define IMP_GCC_PUSH_POP(x)

#define IMP_COMPILER_ENABLE_WARNINGS                                  \
  IMP_VC_PRAGMA(warning(push)) IMP_VC_PRAGMA(warning(disable : 4273)) \
      IMP_VC_PRAGMA(warning(disable : 4244))                          \
      IMP_VC_PRAGMA(warning(disable : 4068))                          \
      IMP_VC_PRAGMA(warning(disable : 4297))

#define IMP_COMPILER_DISABLE_WARNINGS IMP_VC_PRAGMA(warning(pop))

#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS

#else
#define IMP_COMPILER_ENABLE_WARNINGS
#define IMP_COMPILER_DISABLE_WARNINGS
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

#if defined(__GNUC__) || defined(__clang__)
#define IMP_DEPRECATED_ATTRIBUTE __attribute__((deprecated))
#else
#define IMP_DEPRECATED_ATTRIBUTE
#endif

// likely/unlikely macros provide manual optimization of branch
// prediction - particularly useful for rare cases in scoring
// functions, but use only if you know what you're doing cause
// profiler is better at this in 99.9% of times:
#if defined(__GNUC__)
#define IMP_LIKELY(x)    __builtin_expect (!!(x), 1)
#define IMP_UNLIKELY(x)  __builtin_expect (!!(x), 0)
#else // ifdef __GNUC__
#define IMP_LIKELY(x) x
#define IMP_UNLIKELY(x) x
#endif // ifdef __GNUC__

#if defined(_MSC_VER)
//! platform independent C open() method
#define IMP_C_OPEN _open
#define IMP_C_CLOSE _close
  //! platform independent C flag for open() method
#define IMP_C_OPEN_FLAG(x) _##x
#define IMP_C_OPEN_BINARY _O_BINARY
#else
//! platform independent C open() method
#define IMP_C_OPEN open
#define IMP_C_CLOSE close
//! platform independent C flag for open() method
#define IMP_C_OPEN_FLAG(x) x
#define IMP_C_OPEN_BINARY 0
#endif

#endif /* IMPKERNEL_COMPILER_MACROS_H */
