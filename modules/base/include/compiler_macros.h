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
#define IMP_COMPILER_ENABLE_WARNINGS                                    \
  IMP_PRAGMA(clang diagnostic push)                                     \
    /*IMP_PRAGMA(clang diagnostic warning "-Wall")*/                    \
    /*IMP_PRAGMA(clang diagnostic warning "-Wextra") */                 \
  IMP_PRAGMA(clang diagnostic warning "-Wabi")                          \
    IMP_PRAGMA(clang diagnostic warning "-Waddress-of-temporary")       \
    IMP_PRAGMA(clang diagnostic warning "-Waddress")                    \
    IMP_PRAGMA(clang diagnostic warning "-Waggregate-return")           \
    IMP_PRAGMA(clang diagnostic warning "-Wambiguous-member-template")  \
    IMP_PRAGMA(clang diagnostic warning "-Warc-abi")                    \
    IMP_PRAGMA(clang diagnostic warning "-Warc-non-pod-memaccess")      \
    IMP_PRAGMA(clang diagnostic warning "-Warc-retain-cycles")          \
    IMP_PRAGMA(clang diagnostic warning "-Warc-unsafe-retained-assign") \
    IMP_PRAGMA(clang diagnostic warning "-Warc")                        \
    IMP_PRAGMA(clang diagnostic warning "-Watomic-properties")          \
    IMP_PRAGMA(clang diagnostic warning "-Wattributes")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wavailability")               \
    IMP_PRAGMA(clang diagnostic warning "-Wbad-function-cast")          \
    IMP_PRAGMA(clang diagnostic warning "-Wbind-to-temporary-copy")     \
    IMP_PRAGMA(clang diagnostic warning "-Wbitwise-op-parentheses")     \
    IMP_PRAGMA(clang diagnostic warning "-Wbool-conversions")           \
    IMP_PRAGMA(clang diagnostic warning "-Wbuiltin-macro-redefined")    \
    IMP_PRAGMA(clang diagnostic warning "-Wc++-compat")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wc++0x-compat")               \
    IMP_PRAGMA(clang diagnostic ignored "-Wc++11-extensions")           \
    IMP_PRAGMA(clang diagnostic ignored "-Wc++0x-extensions")           \
    IMP_PRAGMA(clang diagnostic warning "-Wcast-align")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wcast-qual")                  \
    IMP_PRAGMA(clang diagnostic warning "-Wchar-align")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wchar-subscripts")            \
    IMP_PRAGMA(clang diagnostic warning "-Wcomment")                    \
    IMP_PRAGMA(clang diagnostic warning "-Wcomments")                   \
    /* Too many false positives */                                      \
    /*IMP_PRAGMA(clang diagnostic warning "-Wconditional-uninitialized")*/ \
    IMP_PRAGMA(clang diagnostic warning "-Wctor-dtor-privacy")          \
    IMP_PRAGMA(clang diagnostic warning "-Wcustom-atomic-properties")   \
    IMP_PRAGMA(clang diagnostic warning "-Wdeclaration-after-statement") \
    IMP_PRAGMA(clang diagnostic warning "-Wdefault-arg-special-member") \
    IMP_PRAGMA(clang diagnostic warning "-Wdelegating-ctor-cycles")     \
    IMP_PRAGMA(clang diagnostic warning "-Wdelete-non-virtual-dtor")    \
    IMP_PRAGMA(clang diagnostic warning "-Wdeprecated-declarations")    \
    IMP_PRAGMA(clang diagnostic warning "-Wdeprecated-implementations") \
    IMP_PRAGMA(clang diagnostic warning "-Wdeprecated-writable-strings") \
    IMP_PRAGMA(clang diagnostic warning "-Wdeprecated")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wdisabled-optimization")      \
    IMP_PRAGMA(clang diagnostic warning "-Wdiscard-qual")               \
    IMP_PRAGMA(clang diagnostic warning "-Wdiv-by-zero")                \
    IMP_PRAGMA(clang diagnostic warning "-Wduplicate-method-arg")       \
    IMP_PRAGMA(clang diagnostic warning "-Weffc++")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wempty-body")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wendif-labels")               \
    IMP_PRAGMA(clang diagnostic ignored "-Wexit-time-destructors")      \
    IMP_PRAGMA(clang diagnostic warning "-Wextra-tokens")               \
    IMP_PRAGMA(clang diagnostic warning "-Wformat-extra-args")          \
    IMP_PRAGMA(clang diagnostic warning "-Wformat-nonliteral")          \
    IMP_PRAGMA(clang diagnostic warning "-Wformat-zero-length")         \
    IMP_PRAGMA(clang diagnostic warning "-Wformat")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wformat=2")                   \
    IMP_PRAGMA(clang diagnostic warning "-Wfour-char-constants")        \
    IMP_PRAGMA(clang diagnostic ignored "-Wglobal-constructors")        \
    IMP_PRAGMA(clang diagnostic warning "-Wgnu-designator")             \
    IMP_PRAGMA(clang diagnostic warning "-Wgnu")                        \
    IMP_PRAGMA(clang diagnostic warning "-Wheader-hygiene")             \
    IMP_PRAGMA(clang diagnostic warning "-Widiomatic-parentheses")      \
    IMP_PRAGMA(clang diagnostic warning "-Wignored-qualifiers")         \
    IMP_PRAGMA(clang diagnostic warning "-Wimplicit-atomic-properties") \
    IMP_PRAGMA(clang diagnostic warning "-Wimplicit-function-declaration") \
    IMP_PRAGMA(clang diagnostic warning "-Wimplicit-int")               \
    IMP_PRAGMA(clang diagnostic warning "-Wimplicit")                   \
    IMP_PRAGMA(clang diagnostic warning "-Wimport")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wincompatible-pointer-types") \
    IMP_PRAGMA(clang diagnostic warning "-Winit-self")                  \
    IMP_PRAGMA(clang diagnostic warning "-Winitializer-overrides")      \
    IMP_PRAGMA(clang diagnostic warning "-Winline")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wint-to-pointer-cast")        \
    IMP_PRAGMA(clang diagnostic warning "-Winvalid-offsetof")           \
    IMP_PRAGMA(clang diagnostic warning "-Winvalid-pch")                \
    IMP_PRAGMA(clang diagnostic warning "-Wlarge-by-value-copy")        \
    IMP_PRAGMA(clang diagnostic warning "-Wliteral-range")              \
    IMP_PRAGMA(clang diagnostic warning "-Wlocal-type-template-args")   \
    IMP_PRAGMA(clang diagnostic warning "-Wlogical-op-parentheses")     \
    IMP_PRAGMA(clang diagnostic warning "-Wlong-long")                  \
    IMP_PRAGMA(clang diagnostic warning "-Wmain")                       \
    IMP_PRAGMA(clang diagnostic warning "-Wmicrosoft")                  \
    IMP_PRAGMA(clang diagnostic warning "-Wmismatched-tags")            \
    IMP_PRAGMA(clang diagnostic warning "-Wmissing-braces")             \
    IMP_PRAGMA(clang diagnostic warning "-Wmissing-declarations")       \
    IMP_PRAGMA(clang diagnostic warning "-Wmissing-field-initializers") \
    IMP_PRAGMA(clang diagnostic warning "-Wmissing-format-attribute")   \
    IMP_PRAGMA(clang diagnostic warning "-Wmissing-include-dirs")       \
    IMP_PRAGMA(clang diagnostic ignored "-Wmissing-noreturn")           \
    IMP_PRAGMA(clang diagnostic warning "-Wmost")                       \
    IMP_PRAGMA(clang diagnostic warning "-Wmultichar")                  \
    IMP_PRAGMA(clang diagnostic warning "-Wnested-externs")             \
    IMP_PRAGMA(clang diagnostic warning "-Wnewline-eof")                \
    IMP_PRAGMA(clang diagnostic warning "-Wnon-gcc")                    \
    IMP_PRAGMA(clang diagnostic warning "-Wnon-virtual-dtor")           \
    /*IMP_PRAGMA(clang diagnostic ignored "-Wnonfragile-abi2")*/        \
    IMP_PRAGMA(clang diagnostic warning "-Wnonnull")                    \
    IMP_PRAGMA(clang diagnostic warning "-Wnonportable-cfstrings")      \
    IMP_PRAGMA(clang diagnostic warning "-Wnull-dereference")           \
    IMP_PRAGMA(clang diagnostic warning "-Wobjc-nonunified-exceptions") \
    IMP_PRAGMA(clang diagnostic warning "-Wold-style-cast")             \
    IMP_PRAGMA(clang diagnostic warning "-Wold-style-definition")       \
    IMP_PRAGMA(clang diagnostic warning "-Wout-of-line-declaration")    \
    IMP_PRAGMA(clang diagnostic warning "-Woverflow")                   \
    IMP_PRAGMA(clang diagnostic warning "-Woverlength-strings")         \
    IMP_PRAGMA(clang diagnostic warning "-Woverloaded-virtual")         \
    IMP_PRAGMA(clang diagnostic warning "-Wpacked")                     \
    IMP_PRAGMA(clang diagnostic ignored "-Wpadded")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wparentheses")                \
    IMP_PRAGMA(clang diagnostic warning "-Wpointer-arith")              \
    IMP_PRAGMA(clang diagnostic warning "-Wpointer-to-int-cast")        \
    IMP_PRAGMA(clang diagnostic warning "-Wprotocol")                   \
    IMP_PRAGMA(clang diagnostic warning "-Wreadonly-setter-attrs")      \
    IMP_PRAGMA(clang diagnostic warning "-Wredundant-decls")            \
    IMP_PRAGMA(clang diagnostic warning "-Wreorder")                    \
    IMP_PRAGMA(clang diagnostic warning "-Wreturn-type")                \
    IMP_PRAGMA(clang diagnostic warning "-Wself-assign")                \
    IMP_PRAGMA(clang diagnostic warning "-Wsemicolon-before-method-body") \
    IMP_PRAGMA(clang diagnostic warning "-Wsequence-point")             \
    /* We should turn these on, but there are too may warnings.*/       \
    /*IMP_PRAGMA(clang diagnostic warning "-Wshadow")*/                 \
    IMP_PRAGMA(clang diagnostic ignored "-Wshorten-64-to-32")           \
    IMP_PRAGMA(clang diagnostic warning "-Wsign-compare")               \
    IMP_PRAGMA(clang diagnostic ignored "-Wsign-conversion")            \
    IMP_PRAGMA(clang diagnostic warning "-Wsign-promo")                 \
    IMP_PRAGMA(clang diagnostic warning "-Wsizeof-array-argument")      \
    IMP_PRAGMA(clang diagnostic warning "-Wstack-protector")            \
    IMP_PRAGMA(clang diagnostic warning "-Wstrict-aliasing")            \
    IMP_PRAGMA(clang diagnostic warning "-Wstrict-overflow")            \
    IMP_PRAGMA(clang diagnostic warning "-Wstrict-prototypes")          \
    IMP_PRAGMA(clang diagnostic warning "-Wstrict-selector-match")      \
    IMP_PRAGMA(clang diagnostic warning "-Wsuper-class-method-mismatch") \
    IMP_PRAGMA(clang diagnostic warning "-Wswitch-default")             \
/* This is just a dumb warning, provided for gcc compat.*/              \
IMP_PRAGMA(clang diagnostic ignored "-Wswitch-enum")                    \
    IMP_PRAGMA(clang diagnostic warning "-Wswitch")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wsynth")                      \
    IMP_PRAGMA(clang diagnostic warning "-Wtautological-compare")       \
    IMP_PRAGMA(clang diagnostic warning "-Wtrigraphs")                  \
    IMP_PRAGMA(clang diagnostic warning "-Wtype-limits")                \
    IMP_PRAGMA(clang diagnostic warning "-Wundeclared-selector")        \
    IMP_PRAGMA(clang diagnostic warning "-Wuninitialized")              \
    IMP_PRAGMA(clang diagnostic ignored "-Wunknown-pragmas")            \
    IMP_PRAGMA(clang diagnostic warning "-Wunnamed-type-template-args") \
    IMP_PRAGMA(clang diagnostic warning "-Wunneeded-internal-declaration") \
    IMP_PRAGMA(clang diagnostic warning "-Wunneeded-member-function")   \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-argument")            \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-exception-parameter") \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-function")            \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-label")               \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-member-function")     \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-parameter")           \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-value")               \
    IMP_PRAGMA(clang diagnostic warning "-Wunused-variable")            \
    IMP_PRAGMA(clang diagnostic warning "-Wunused")                     \
    IMP_PRAGMA(clang diagnostic warning "-Wused-but-marked-unused")     \
    IMP_PRAGMA(clang diagnostic warning "-Wvariadic-macros")            \
    IMP_PRAGMA(clang diagnostic warning "-Wvector-conversions")         \
    IMP_PRAGMA(clang diagnostic warning "-Wvla")                        \
    IMP_PRAGMA(clang diagnostic warning "-Wvolatile-register-var")      \
    IMP_PRAGMA(clang diagnostic warning "-Wwrite-strings")              \
  /* Most of these are stupid uses of floats instead of doubles. I don't
     want to fix them all now. For some reason this needs to be last.*/ \
IMP_PRAGMA(clang diagnostic ignored "-Wconversion")                     \
IMP_PRAGMA(clang diagnostic ignored "-Wc++11-compat")


#define IMP_HELPER_MACRO_PUSH_WARNINGS \
  IMP_PRAGMA(clang diagnostic push)    \
  IMP_PRAGMA(clang diagnostic ignored "-Wunused-member-function")

#define IMP_HELPER_MACRO_POP_WARNINGS \
  IMP_PRAGMA(clang diagnostic pop)

/*  IMP_PRAGMA(clang diagnostic warning "-Wall")                          \
    IMP_PRAGMA(clang diagnostic warning "-Weverything")                 \
    IMP_PRAGMA(clang diagnostic ignored "-Wpadded")                     \
    IMP_PRAGMA(clang diagnostic ignored "-Wc++11-extensions")           \
    IMP_PRAGMA(clang diagnostic ignored "-Wunknown-pragmas")            \
    IMP_PRAGMA(clang diagnostic ignored "-Wc++98-compat")*/

#define IMP_COMPILER_DISABLE_WARNINGS           \
  IMP_PRAGMA(clang diagnostic pop)

#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
  "-Wstrict-aliasing=2",
  -fno-operator-names",]*/
#if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#define IMP_GCC_PUSH_POP(x) IMP_PRAGMA(x)
#define IMP_GCC_CXX0X_COMPAT                            \
  IMP_PRAGMA(GCC diagnostic ignored "-Wc++0x-compat")
#define IMP_GCC_PROTOTYPES                                      \
  IMP_PRAGMA(GCC diagnostic warning "-Wmissing-declarations")

#define IMP_HELPER_MACRO_PUSH_WARNINGS \
  IMP_PRAGMA(GCC diagnostic push)

#define IMP_HELPER_MACRO_POP_WARNINGS \
  IMP_PRAGMA(GCC diagnostic pop)


#else
#define IMP_GCC_PUSH_POP(x)
#define IMP_GCC_CXX0X_COMPAT
#define IMP_GCC_PROTOTYPES
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

#define IMP_COMPILER_ENABLE_WARNINGS                            \
  IMP_GCC_PUSH_POP(GCC diagnostic push)                         \
  IMP_PRAGMA(GCC diagnostic warning "-Wall")                    \
    IMP_PRAGMA(GCC diagnostic warning "-Wextra")                \
    IMP_PRAGMA(GCC diagnostic warning "-Winit-self")            \
    IMP_PRAGMA(GCC diagnostic warning "-Wcast-align")           \
    IMP_PRAGMA(GCC diagnostic ignored "-Wunknown-pragmas")      \
    IMP_PRAGMA(GCC diagnostic warning "-Woverloaded-virtual")   \
    IMP_PRAGMA(GCC diagnostic warning "-Wundef")                \
    IMP_GCC_PROTOTYPES                                          \
    IMP_GCC_CXX0X_COMPAT

#define IMP_COMPILER_DISABLE_WARNINGS           \
  IMP_GCC_PUSH_POP(GCC diagnostic pop)

#else
#define IMP_COMPILER_ENABLE_WARNINGS
#define IMP_COMPILER_DISABLE_WARNINGS
#define IMP_HELPER_MACRO_PUSH_WARNINGS
#define IMP_HELPER_MACRO_POP_WARNINGS
#endif

#endif  /* IMPBASE_BASE_COMPILER_MACROS_H */
