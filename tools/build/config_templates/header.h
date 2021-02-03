/*
 * \file %(filename)s
 * \brief Provide macros to mark functions and classes as exported
 *        from a DLL/.so, and to set up namespaces
 *
 * When building the module, %(cppprefix)s_EXPORTS should be defined, and when
 * using the module externally, it should not be. Classes and functions
 * declared in the module's headers should then be marked with
 * %(cppprefix)sEXPORT if they are intended to be part of the API and
 * they are not defined entirely in a header.
 *
 * The Windows build environment requires applications to mark exports in
 * this way; we use the same markings to set the visibility of ELF symbols
 * if we have compiler support.
 *
 * All code in this module should live in the %(namespace)s namespace.
 * This is simply achieved by wrapping things with the
 * %(cppprefix)s_BEGIN_NAMESPACE and %(cppprefix)s_END_NAMESPACE macros.
 * There are similar macros for module code that is designed to be for
 * internal use only.
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef %(cppprefix)s_CONFIG_H
#define %(cppprefix)s_CONFIG_H

#include <IMP/kernel_config.h>
#include <string>

#ifdef _MSC_VER
#define NOMINMAX

#ifdef %(cppprefix)s_EXPORTS
#define %(cppprefix)sEXPORT __declspec(dllexport)
#else  // EXPORTS
#define %(cppprefix)sEXPORT __declspec(dllimport)
#endif  // EXPORTS

#else  // _MSC_VER

#ifdef GCC_VISIBILITY
#define %(cppprefix)sEXPORT __attribute__((visibility("default")))
#else  // GCC_VISIBILITY
#define %(cppprefix)sEXPORT
#endif  // GCC_VISIBILITY

#endif  // _MSC_VER

#if defined(_MSC_VER) && !defined(SWIG)
#ifdef %(cppprefix)s_EXPORTS

#define %(cppprefix)s_EXPORT_TEMPLATE(name)     \
  template class __declspec(dllexport) name

#else  //EXPORTS

#define %(cppprefix)s_EXPORT_TEMPLATE(name)     \
  template class __declspec(dllimport) name

#endif  // EXPORTS

#else  // MSC and SWIG
#define %(cppprefix)s_EXPORT_TEMPLATE(name) IMP_REQUIRE_SEMICOLON_NAMESPACE

#endif  // MSC and SWIG

#if !defined(SWIG) && !defined(IMP_DOXYGEN)

#define %(cppprefix)s_BEGIN_NAMESPACE \
  IMP_COMPILER_ENABLE_WARNINGS %(begin_ns)s

#define %(cppprefix)s_END_NAMESPACE %(end_ns)s \
IMP_COMPILER_DISABLE_WARNINGS

#define %(cppprefix)s_BEGIN_INTERNAL_NAMESPACE %(cppprefix)s_BEGIN_NAMESPACE \
  namespace internal {

#define %(cppprefix)s_END_INTERNAL_NAMESPACE } %(cppprefix)s_END_NAMESPACE

#else  // SWIG and DOXYGEN
#define %(cppprefix)s_BEGIN_NAMESPACE %(begin_ns)s

#define %(cppprefix)s_END_NAMESPACE %(end_ns)s

#define %(cppprefix)s_BEGIN_INTERNAL_NAMESPACE %(cppprefix)s_BEGIN_NAMESPACE\
  namespace internal {

#define %(cppprefix)s_END_INTERNAL_NAMESPACE } %(cppprefix)s_END_NAMESPACE

#endif  // SWIG AND DOXYGEN

%(cppdefines)s

// functions are defined explicitly for SWIG
%(begin_ns)s
/** \name Standard module functions
  All \imp modules have a set of standard functions to help get information
  about the module and about files associated with the module.
  @{
  */
#if !defined(SWIG)
    %(cppprefix)sEXPORT std::string get_module_version();
#endif

#if !defined(SWIG)
    // SWIG will whine about duplicate definitions of function
    inline std::string get_module_name() { return "%(namespace)s"; }
#endif

    //! Return the full path to one of this module's data files
    /** To read the data file "data_library" that was placed in the \c data
        directory of this module, do something like
        \code
        std::ifstream in(%(namespace)s::get_data_path("data_library"));
        \endcode
        This will ensure that the code works both when %%IMP is installed or
        if used via the \c setup_environment.sh script.

        \note Each module has its own data directory, so be sure to use
              this function from the correct module.
    */
#if !defined(SWIG)
    %(cppprefix)sEXPORT std::string get_data_path(std::string file_name);
#endif

    //! Return the full path to one of this module's example files
    /** To read the example file "example_protein.pdb" that was placed
        in the \c examples directory of this module, do something like
        \code
        std::ifstream in(%(namespace)s::get_example_path("example_protein.pdb"));
        \endcode
        This will ensure that the code works both when %%IMP is installed or
        if used via the \c setup_environment.sh script.

        \note Each module has its own example directory, so be sure to use
              this function from the correct module.
    */
#if !defined(SWIG)
    %(cppprefix)sEXPORT std::string get_example_path(std::string file_name);
#endif
    /** @} */


%(end_ns)s  //namespace

%(showable)s

#include <IMP/compiler_macros.h>

#endif /* %(cppprefix)s_CONFIG_H */

// Here so it is always parsed

#ifdef %(cppprefix)s_DEPRECATED_HEADER
#undef %(cppprefix)s_DEPRECATED_HEADER
#undef %(cppprefix)s_DEPRECATED_VALUE_DEF
#undef %(cppprefix)s_DEPRECATED_VALUE_DECL
#undef %(cppprefix)s_DEPRECATED_OBJECT_DEF
#undef %(cppprefix)s_DEPRECATED_OBJECT_DECL
#undef %(cppprefix)s_DEPRECATED_FUNCTION_DEF
#undef %(cppprefix)s_DEPRECATED_FUNCTION_DECL
#undef %(cppprefix)s_DEPRECATED_METHOD_DEF
#undef %(cppprefix)s_DEPRECATED_METHOD_DECL
#undef %(cppprefix)s_DEPRECATED_MACRO
#undef %(cppprefix)s_SHOW_WARNINGS
#endif

// the central modules we can update easily, so don't warn in them
#if defined( %(cppprefix)s_COMPILATION)                                 \
  || defined(SWIG) || defined(IMP_SWIG_WRAPPER)                         \
  || defined( %(cppprefix)s_ALL) || defined(IMP_DOXYGEN)                \
  || defined(IMPBASE_COMPILATION) || defined(IMPKERNEL_COMPILATION)     \
  || defined(IMPCORE_COMPILATION) || defined(IMPATOM_COMPILATION)       \
  || defined(IMPSTATISTICS_COMPILATION) || defined(IMPDOMINO_COMPILATION) \
  || defined(IMPCONTAINER_COMPILATION) || defined(IMPDISPLAY_COMPILATION) \
  || defined(IMPSCOREFUNCTOR_COMPILATION) || defined(IMPRMF_COMPILATION) \
  || defined(IMPGSL_COMPILATION)
#define %(cppprefix)s_SHOW_WARNINGS 0
#else
#define %(cppprefix)s_SHOW_WARNINGS 1
#endif

// suppress header warnings with all header, SWIG wrapper and in the module
#if %(cppprefix)s_SHOW_WARNINGS
#define %(cppprefix)s_DEPRECATED_HEADER(version, help_message)          \
    IMP_PRAGMA(message(__FILE__ " is deprecated: " help_message))
#define %(cppprefix)s_DEPRECATED_VALUE_DECL(version)    \
  IMP_DEPRECATED_ATTRIBUTE
#define %(cppprefix)s_DEPRECATED_OBJECT_DECL(version)    \
  IMP_DEPRECATED_ATTRIBUTE
#define %(cppprefix)s_DEPRECATED_FUNCTION_DECL(version) \
  IMP_DEPRECATED_ATTRIBUTE
#define %(cppprefix)s_DEPRECATED_METHOD_DECL(version) \
  IMP_DEPRECATED_ATTRIBUTE

#else //%(cppprefix)s_SHOW_WARNINGS
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_HEADER(version, help_message) \
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_VALUE_DECL(version)
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_OBJECT_DECL(version)
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_FUNCTION_DECL(version)
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_METHOD_DECL(version)

#endif // %(cppprefix)s_SHOW_WARNINGS

// only warn about it in the all inclusion to cut down on copies
#if !defined(IMP_ALL) || defined(SWIG) || defined(IMP_DOXYGEN)          \
  || defined(IMPBASE_COMPILATION) || defined(IMPKERNEL_COMPILATION)     \
  || defined(IMPCORE_COMPILATION) || defined(IMPATOM_COMPILATION)       \
  || defined(IMPSTATISTICS_COMPILATION) || defined(IMPDOMINO_COMPILATION) \
  || defined(IMPCONTAINER_COMPILATION) || defined(IMPDISPLAY_COMPILATION) \
  || defined(IMPSCOREFUNCTOR_COMPILATION)
/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_MACRO(version, message)

#else
#define %(cppprefix)s_DEPRECATED_MACRO(version, message)        \
  IMP_DEPRECATED_MACRO(version, message)
#endif

/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_VALUE_DEF(version, message) \
  IMP_DEPRECATED_VALUE_RUNTIME_WARNING(version, message)

/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_OBJECT_DEF(version, message) \
  IMP_DEPRECATED_OBJECT_RUNTIME_WARNING(version, message)

/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_FUNCTION_DEF(version, message) \
  IMP_DEPRECATED_FUNCTION_RUNTIME_WARNING(version, message)

/** See [deprecation support](@ref deprecation). */
#define %(cppprefix)s_DEPRECATED_METHOD_DEF(version, message) \
  IMP_DEPRECATED_METHOD_RUNTIME_WARNING(version, message)


#include <IMP/compiler_macros.h>

#if defined(IMP_EXECUTABLE) && !defined(IMP_EXECUTABLE_WARNINGS)
IMP_COMPILER_ENABLE_WARNINGS
#define IMP_EXECUTABLE_WARNINGS
#endif
