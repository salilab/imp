/**
 *  \file RMF/base/compiler_macros.h
 *  \brief Various compiler workarounds
 *
 *  Copyright 2007-2013 RMF Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_COMPILER_MACROS_H
#define RMF_INTERNAL_COMPILER_MACROS_H

#define RMF_STRINGIFY(x) #x

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

#ifdef __clang__
#define RMF_COMPILER_HAS_OVERRIDE 1
#elif defined(__GNUC__) && __cplusplus >= 201103L
// probably should be finer here
#define RMF_COMPILER_HAS_OVERRIDE 1
#else
#define RMF_COMPILER_HAS_OVERRIDE 0
#endif

#if RMF_COMPILER_HAS_OVERRIDE
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

#if RMF_COMPILER_HAS_FINAL
#define RMF_FINAL final
#else
#define RMF_FINAL
#endif

 #if defined(__clang__) || defined(__GNUC__)
#define RMF_PRAGMA(x) _Pragma(RMF_STRINGIFY(x))
#elif defined(_MSC_VER)
#define RMF_PRAGMA(x) __pragma(x)
#else
#define RMF_PRAGMA(x)
#endif

// Compiler-specific pragma support
#if defined(__clang__)
#define RMF_CLANG_PRAGMA(x) RMF_PRAGMA(clang x)
#define RMF_GCC_PRAGMA(x)
#define RMF_VC_PRAGMA(x)

#elif defined (__GNUC__)
#define RMF_CLANG_PRAGMA(x)
#define RMF_GCC_PRAGMA(x) RMF_PRAGMA(GCC x)
#define RMF_VC_PRAGMA(x)

#elif defined(_MSC_VER)
#define RMF_CLANG_PRAGMA(x)
#define RMF_GCC_PRAGMA(x)
#define RMF_VC_PRAGMA(x) RMF_PRAGMA(x)

#else
#define RMF_CLANG_PRAGMA(x)
#define RMF_GCC_PRAGMA(x)
#define RMF_VC_PRAGMA(x)
#endif

// Support for pushing and popping of warning state
#if defined(__clang__)
#define RMF_PUSH_WARNINGS RMF_CLANG_PRAGMA( diagnostic push)
#define RMF_POP_WARNINGS RMF_CLANG_PRAGMA( diagnostic pop)

#elif defined (__GNUC__)
#if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#define RMF_PUSH_WARNINGS RMF_GCC_PRAGMA( diagnostic push)
#define RMF_POP_WARNINGS RMF_GCC_PRAGMA( diagnostic pop)

#else
#define RMF_PUSH_WARNINGS
#define RMF_POP_WARNINGS

#endif

#elif defined(_MSC_VER)
#define RMF_PUSH_WARNINGS RMF_VC_PRAGMA(warning(push))
#define RMF_POP_WARNINGS RMF_VC_PRAGMA(warning(pop))

#else
#define RMF_PUSH_WARNINGS
#define RMF_POP_WARNINGS

#endif


// Turning on and off compiler-specific warnings
#ifdef __clang__
#define RMF_COMPILER_WARNINGS                                            \
    /*RMF_CLANG_PRAGMA( diagnostic warning "-Wall")*/                    \
    /*RMF_CLANG_PRAGMA( diagnostic warning "-Wextra") */                 \
  RMF_CLANG_PRAGMA( diagnostic warning "-Wabi")                          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Waddress-of-temporary")       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Waddress")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Waggregate-return")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wambiguous-member-template")  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Warc-abi")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Warc-non-pod-memaccess")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Warc-retain-cycles")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Warc-unsafe-retained-assign") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Warc")                        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Watomic-properties")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wattributes")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wavailability")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wbad-function-cast")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wbind-to-temporary-copy")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wbitwise-op-parentheses")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wbool-conversions")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wbuiltin-macro-redefined")    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wc++-compat")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wc++0x-compat")               \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wc++11-extensions")           \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wc++0x-extensions")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wcast-align")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wcast-qual")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wchar-align")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wchar-subscripts")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wcomment")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wcomments")                   \
    /* Too many false positives */                                      \
    /*RMF_CLANG_PRAGMA( diagnostic warning "-Wconditional-uninitialized")*/ \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wctor-dtor-privacy")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wcustom-atomic-properties")   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdeclaration-after-statement") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdefault-arg-special-member") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdelegating-ctor-cycles")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdelete-non-virtual-dtor")    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-declarations")    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-implementations") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdeprecated-writable-strings") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdeprecated")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdisabled-optimization")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdiscard-qual")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wdiv-by-zero")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wduplicate-method-arg")       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Weffc++")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wempty-body")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wendif-labels")               \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wexit-time-destructors")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wextra-tokens")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wformat-extra-args")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wformat-nonliteral")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wformat-zero-length")         \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wformat")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wformat=2")                   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wfour-char-constants")        \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wglobal-constructors")        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wgnu-designator")             \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wgnu")                        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wheader-hygiene")             \
    RMF_CLANG_PRAGMA( diagnostic warning "-Widiomatic-parentheses")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wignored-qualifiers")         \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wimplicit-atomic-properties") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wimplicit-function-declaration") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wimplicit-int")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wimplicit")                   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wimport")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wincompatible-pointer-types") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Winit-self")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Winitializer-overrides")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Winline")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wint-to-pointer-cast")        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Winvalid-offsetof")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Winvalid-pch")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wlarge-by-value-copy")        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wliteral-range")              \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wlocal-type-template-args")   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wlogical-op-parentheses")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wlong-long")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmain")                       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmicrosoft")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmismatched-tags")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmissing-braces")             \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmissing-declarations")       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmissing-field-initializers") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmissing-format-attribute")   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmissing-include-dirs")       \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wmissing-noreturn")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmost")                       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wmultichar")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnested-externs")             \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnewline-eof")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnon-gcc")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnon-virtual-dtor")           \
    /*RMF_CLANG_PRAGMA( diagnostic ignored "-Wnonfragile-abi2")*/        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnonnull")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnonportable-cfstrings")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wnull-dereference")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wobjc-nonunified-exceptions") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wold-style-cast")             \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wold-style-definition")       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wout-of-line-declaration")    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Woverflow")                   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Woverlength-strings")         \
    RMF_CLANG_PRAGMA( diagnostic warning "-Woverloaded-virtual")         \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wpacked")                     \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wpadded")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wparentheses")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wpointer-arith")              \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wpointer-to-int-cast")        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wprotocol")                   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wreadonly-setter-attrs")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wredundant-decls")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wreorder")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wreturn-type")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wself-assign")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsemicolon-before-method-body") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsequence-point")             \
    /* We should turn these on, but there are too may warnings.*/       \
    /*RMF_CLANG_PRAGMA( diagnostic warning "-Wshadow")*/                 \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wshorten-64-to-32")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsign-compare")               \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wsign-conversion")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsign-promo")                 \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsizeof-array-argument")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wstack-protector")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wstrict-aliasing")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wstrict-overflow")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wstrict-prototypes")          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wstrict-selector-match")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsuper-class-method-mismatch") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wswitch-default")             \
/* This is just a dumb warning, provided for gcc compat.*/              \
RMF_CLANG_PRAGMA( diagnostic ignored "-Wswitch-enum")                    \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wswitch")                     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wsynth")                      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wtautological-compare")       \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wtrigraphs")                  \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wtype-limits")                \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wundeclared-selector")        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wuninitialized")              \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunknown-pragmas")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunnamed-type-template-args") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunneeded-internal-declaration") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunneeded-member-function")   \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-argument")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-exception-parameter") \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-function")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-label")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-member-function")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-parameter")           \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-value")               \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused-variable")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wunused")                     \
    /* This is getting triggered with swig for some reason */           \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wused-but-marked-unused")     \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wvariadic-macros")            \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wvector-conversions")         \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wvla")                        \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wvolatile-register-var")      \
    RMF_CLANG_PRAGMA( diagnostic warning "-Wwrite-strings")              \
  /* Most of these are stupid uses of floats instead of doubles. I don't
     want to fix them all now. For some reason this needs to be last.*/ \
RMF_CLANG_PRAGMA( diagnostic ignored "-Wconversion")                     \
RMF_CLANG_PRAGMA( diagnostic ignored "-Wc++11-compat") \
RMF_CLANG_PRAGMA( diagnostic ignored "-Wunused-member-function")


/*  RMF_CLANG_PRAGMA( diagnostic warning "-Wall")                          \
    RMF_CLANG_PRAGMA( diagnostic warning "-Weverything")                 \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wpadded")                     \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wc++11-extensions")           \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wunknown-pragmas")            \
    RMF_CLANG_PRAGMA( diagnostic ignored "-Wc++98-compat")*/


#elif defined(__GNUC__)

/*ret+=["-Wno-deprecated",
  "-Wstrict-aliasing=2",
  -fno-operator-names",]*/
#  if __GNUC__ > 4 || __GNUC_MINOR__ >=6
#    define RMF_GCC_CXX0X_COMPAT                            \
       RMF_GCC_PRAGMA( diagnostic ignored "-Wc++0x-compat")
#    define RMF_GCC_PROTOTYPES                                      \
       RMF_GCC_PRAGMA( diagnostic warning "-Wmissing-declarations")


#  else
#    define RMF_GCC_CXX0X_COMPAT
#    define RMF_GCC_PROTOTYPES
#  endif


#  define RMF_COMPILER_WARNINGS                            \
    RMF_GCC_PRAGMA( diagnostic warning "-Wall")                    \
    RMF_GCC_PRAGMA( diagnostic warning "-Wextra")                \
    RMF_GCC_PRAGMA( diagnostic warning "-Winit-self")            \
    RMF_GCC_PRAGMA( diagnostic warning "-Wcast-align")           \
    RMF_GCC_PRAGMA( diagnostic warning "-Woverloaded-virtual")   \
    RMF_GCC_PRAGMA( diagnostic warning "-Wundef")                \
    RMF_GCC_PROTOTYPES                                          \
    RMF_GCC_CXX0X_COMPAT

#elif defined(_MSC_VER)
#  define RMF_COMPILER_WARNINGS \
    RMF_VC_PRAGMA(warning(disable:4275)) \
    RMF_VC_PRAGMA(warning(disable:4251))

#else
#  define RMF_COMPILER_WARNINGS

#endif

#define RMF_ENABLE_WARNINGS \
  RMF_PUSH_WARNINGS \
  RMF_COMPILER_WARNINGS

#define RMF_DISABLE_WARNINGS RMF_POP_WARNINGS


#endif  /* RMF_INTERNAL_COMPILER_MACROS_H */
