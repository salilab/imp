/**
 *  \file IMP/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_BASE_COMPILER_MACROS_H
#define IMPBASE_BASE_COMPILER_MACROS_H

#include <boost/detail/endian.hpp>

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

#if IMP_COMPILER_HAS_OVERRIDE
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

#if IMP_COMPILER_HAS_FINAL
#define IMP_FINAL final
#else
#define IMP_FINAL
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

#ifndef IMP_DOXYGEN
#if defined(BOOST_LITTLE_ENDIAN)
#define IMP_LITTLE_ENDIAN
#else
#define IMP_BIG_ENDIAN
#endif
#endif

#ifdef __clang__

#define IMP_GCC_PUSH_POP(x)

#define IMP_COMPILER_ENABLE_WARNINGS                                    \
  IMP_CLANG_PRAGMA( diagnostic push)                                     \
    /*IMP_CLANG_PRAGMA( diagnostic warning "-Wall")*/                    \
    /*IMP_CLANG_PRAGMA( diagnostic warning "-Wextra") */                 \
  IMP_CLANG_PRAGMA( diagnostic warning "-Wabi")                          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Waddress-of-temporary")       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Waddress")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Waggregate-return")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wambiguous-member-template")  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Warc-abi")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Warc-non-pod-memaccess")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Warc-retain-cycles")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Warc-unsafe-retained-assign") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Warc")                        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Watomic-properties")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wattributes")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wavailability")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wbad-function-cast")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wbind-to-temporary-copy")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wbitwise-op-parentheses")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wbool-conversions")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wbuiltin-macro-redefined")    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wc++-compat")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wc++0x-compat")               \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wc++11-extensions")           \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wc++0x-extensions")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wcast-align")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wcast-qual")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wchar-align")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wchar-subscripts")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wcomment")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wcomments")                   \
    /* Too many false positives */                                      \
    /*IMP_CLANG_PRAGMA( diagnostic warning "-Wconditional-uninitialized")*/ \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wctor-dtor-privacy")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wcustom-atomic-properties")   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdeclaration-after-statement") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdefault-arg-special-member") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdelegating-ctor-cycles")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdelete-non-virtual-dtor")    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-declarations")    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-implementations") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-writable-strings") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdeprecated")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdisabled-optimization")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdiscard-qual")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wdiv-by-zero")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wduplicate-method-arg")       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Weffc++")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wempty-body")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wendif-labels")               \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wexit-time-destructors")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wextra-tokens")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wformat-extra-args")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wformat-nonliteral")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wformat-zero-length")         \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wformat")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wformat=2")                   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wfour-char-constants")        \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wglobal-constructors")        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wgnu-designator")             \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wgnu")                        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wheader-hygiene")             \
    IMP_CLANG_PRAGMA( diagnostic warning "-Widiomatic-parentheses")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wignored-qualifiers")         \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wimplicit-atomic-properties") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wimplicit-function-declaration") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wimplicit-int")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wimplicit")                   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wimport")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wincompatible-pointer-types") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Winit-self")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Winitializer-overrides")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Winline")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wint-to-pointer-cast")        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Winvalid-offsetof")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Winvalid-pch")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wlarge-by-value-copy")        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wliteral-range")              \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wlocal-type-template-args")   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wlogical-op-parentheses")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wlong-long")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmain")                       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmicrosoft")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmismatched-tags")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmissing-braces")             \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmissing-declarations")       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmissing-field-initializers") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmissing-format-attribute")   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmissing-include-dirs")       \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wmissing-noreturn")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmost")                       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wmultichar")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnested-externs")             \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnewline-eof")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnon-gcc")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnon-virtual-dtor")           \
    /*IMP_CLANG_PRAGMA( diagnostic ignored "-Wnonfragile-abi2")*/        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnonnull")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnonportable-cfstrings")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wnull-dereference")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wobjc-nonunified-exceptions") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wold-style-cast")             \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wold-style-definition")       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wout-of-line-declaration")    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Woverflow")                   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Woverlength-strings")         \
    IMP_CLANG_PRAGMA( diagnostic warning "-Woverloaded-virtual")         \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wpacked")                     \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wpadded")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wparentheses")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wpointer-arith")              \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wpointer-to-int-cast")        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wprotocol")                   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wreadonly-setter-attrs")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wredundant-decls")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wreorder")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wreturn-type")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wself-assign")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsemicolon-before-method-body") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsequence-point")             \
    /* We should turn these on, but there are too may warnings.*/       \
    /*IMP_CLANG_PRAGMA( diagnostic warning "-Wshadow")*/                 \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wshorten-64-to-32")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsign-compare")               \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wsign-conversion")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsign-promo")                 \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsizeof-array-argument")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wstack-protector")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wstrict-aliasing")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wstrict-overflow")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wstrict-prototypes")          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wstrict-selector-match")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsuper-class-method-mismatch") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wswitch-default")             \
/* This is just a dumb warning, provided for gcc compat.*/              \
IMP_CLANG_PRAGMA( diagnostic ignored "-Wswitch-enum")                    \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wswitch")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wsynth")                      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wtautological-compare")       \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wtrigraphs")                  \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wtype-limits")                \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wundeclared-selector")        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wuninitialized")              \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunknown-pragmas")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunnamed-type-template-args") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunneeded-internal-declaration") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunneeded-member-function")   \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-argument")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-exception-parameter") \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-function")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-label")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-member-function")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-parameter")           \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-value")               \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused-variable")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wunused")                     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wused-but-marked-unused")     \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wvariadic-macros")            \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wvector-conversions")         \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wvla")                        \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wvolatile-register-var")      \
    IMP_CLANG_PRAGMA( diagnostic warning "-Wwrite-strings")              \
  /* Most of these are stupid uses of floats instead of doubles. I don't
     want to fix them all now. For some reason this needs to be last.*/ \
IMP_CLANG_PRAGMA( diagnostic ignored "-Wconversion")                     \
IMP_CLANG_PRAGMA( diagnostic ignored "-Wc++11-compat")


#define IMP_HELPER_MACRO_PUSH_WARNINGS \
  IMP_CLANG_PRAGMA( diagnostic push)    \
  IMP_CLANG_PRAGMA( diagnostic ignored "-Wunused-member-function")

#define IMP_HELPER_MACRO_POP_WARNINGS \
  IMP_CLANG_PRAGMA( diagnostic pop)

/*  IMP_CLANG_PRAGMA( diagnostic warning "-Wall")                          \
    IMP_CLANG_PRAGMA( diagnostic warning "-Weverything")                 \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wpadded")                     \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wc++11-extensions")           \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wunknown-pragmas")            \
    IMP_CLANG_PRAGMA( diagnostic ignored "-Wc++98-compat")*/

#define IMP_COMPILER_DISABLE_WARNINGS           \
  IMP_CLANG_PRAGMA( diagnostic pop)

#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
  "-Wstrict-aliasing=2",
  -fno-operator-names",]*/
#if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#define IMP_GCC_PUSH_POP(x) IMP_PRAGMA(x)
#define IMP_GCC_CXX0X_COMPAT                            \
  IMP_GCC_PRAGMA( diagnostic ignored "-Wc++0x-compat")
#define IMP_GCC_PROTOTYPES                                      \
  IMP_GCC_PRAGMA( diagnostic warning "-Wmissing-declarations")

#define IMP_HELPER_MACRO_PUSH_WARNINGS \
  IMP_GCC_PRAGMA( diagnostic push)

#define IMP_HELPER_MACRO_POP_WARNINGS \
  IMP_GCC_PRAGMA( diagnostic pop)


#else
#define IMP_GCC_PUSH_POP(x)
#define IMP_GCC_CXX0X_COMPAT
#define IMP_GCC_PROTOTYPES
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

#define IMP_COMPILER_ENABLE_WARNINGS                            \
  IMP_GCC_PUSH_POP(GCC diagnostic push)                         \
  IMP_GCC_PRAGMA( diagnostic warning "-Wall")                    \
    IMP_GCC_PRAGMA( diagnostic warning "-Wextra")                \
    IMP_GCC_PRAGMA( diagnostic warning "-Winit-self")            \
    IMP_GCC_PRAGMA( diagnostic warning "-Wcast-align")           \
    IMP_GCC_PRAGMA( diagnostic warning "-Woverloaded-virtual")   \
    IMP_GCC_PRAGMA( diagnostic warning "-Wundef")                \
    IMP_GCC_PROTOTYPES                                          \
    IMP_GCC_CXX0X_COMPAT

#define IMP_COMPILER_DISABLE_WARNINGS           \
  IMP_GCC_PUSH_POP(GCC diagnostic pop)

#elif defined(_MSC_VER)
#define IMP_GCC_PUSH_POP(x)

#define IMP_COMPILER_ENABLE_WARNINGS\
    IMP_VC_PRAGMA(warning(push)) \
    IMP_VC_PRAGMA(warning( disable: 4273 ))

#define IMP_COMPILER_DISABLE_WARNINGS\
    IMP_VC_PRAGMA(warning(pop))

#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS

#else
#define IMP_COMPILER_ENABLE_WARNINGS
#define IMP_COMPILER_DISABLE_WARNINGS
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

#endif  /* IMPBASE_BASE_COMPILER_MACROS_H */
