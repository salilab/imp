/**
 *  \file IMP/base/deprecation_macros.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_DEPRECATION_MACROS_H
#define IMPBASE_DEPRECATION_MACROS_H

#include <IMP/base/base_config.h>
#include "deprecation.h"

/** Used to implement deprecation support.
    See [deprecation support](../devguide.html#devguide_deprecation). */
#define IMP_DEPRECATED_MACRO(version, help_message)                    \
  IMP_PRAGMA(message("This macro is deprecrecated as of IMP " #version \
                     ": " help_message))

/** Used to implement deprecation support.
    See [deprecation support](../devguide.html#devguide_deprecation). */
#define IMP_DEPRECATED_OBJECT_RUNTIME_WARNING(version, help_message)       \
  {                                                                 \
    std::ostringstream oss;                                         \
    oss << "Object " << get_module_name() << "::"                   \
        << get_type_name() << " is deprecated. "                   \
        << help_message << std::endl;                               \
    IMP::base::handle_use_deprecated(oss.str());                    \
  }

/** Used to implement deprecation support.
    See [deprecation support](../devguide.html#devguide_deprecation). */
#define IMP_DEPRECATED_VALUE_RUNTIME_WARNING(version, help_message) \
  {                                                                 \
    std::ostringstream oss;                                         \
    oss << "Class " << get_module_name() << "::"                    \
        << IMP_CURRENT_FUNCTION << " is deprecated. "               \
        << help_message << std::endl;                               \
    IMP::base::handle_use_deprecated(oss.str());                    \
  }

/** Used to implement deprecation support.
    See [deprecation support](../devguide.html#devguide_deprecation). */
#define IMP_DEPRECATED_FUNCTION_RUNTIME_WARNING(version, help_message)       \
  {                                                                     \
    std::ostringstream oss;                                             \
    oss << "Function " << IMP_CURRENT_PRETTY_FUNCTION                   \
        << " is deprecated. "                                           \
        << help_message << std::endl;                                   \
    IMP::base::handle_use_deprecated(oss.str());                        \
  }

/** Used to implement deprecation support.
    See [deprecation support](../devguide.html#devguide_deprecation). */
#define IMP_DEPRECATED_METHOD_RUNTIME_WARNING(version, help_message)    \
  {                                                                     \
    std::ostringstream oss;                                             \
    oss << "Method " << IMP_CURRENT_PRETTY_FUNCTION                     \
        << " is deprecated. "                                           \
        << "WARNING: " << help_message << std::endl;                    \
    IMP::base::handle_use_deprecated(oss.str());                        \
  }

/** \deprecated_at{2.1} Deprecated as of IMP 2.1. Use
    IMPMODULE_DEPRECATED_CLASS_DEF().
 */
#define IMP_DEPRECATED_OBJECT(replacement_classname)                    \
  IMP_DEPRECATED_MACRO(2.1,                                             \
                       "Use IMP@MODULE@_DEPRECATED_OBJECT_DECL()")       \
  if (::IMP::base::internal::                                           \
      get_print_deprecation_message(                                    \
          get_name())) {                                                \
    IMP_WARN(get_name() << " is deprecated "                            \
             << "and should not be used.\nUse "                         \
             << #replacement_classname << " instead."                   \
             << std::endl);                                             \
    ::IMP::base::internal::set_printed_deprecation_message(get_name(), true); \
  }
/** \deprecated_at{2.1} As of IMP 2.1. Use IMPMODULE_DEPRECATED_CLASS_DEF()
    instead
 */
#define IMP_DEPRECATED_CLASS(classname, replacement_classname)                \
  IMP_DEPRECATED_MACRO(                                                       \
      2.1,                                                                    \
      "Use IMP@MODULE@_DEPRECATED_VALUE_DEF "                                 \
      "instead") if (::IMP::base::internal::get_print_deprecation_message(    \
                         #classname)) {                                       \
    IMP_WARN(#classname << " is deprecated "                                  \
                        << "and should not be used.\nUse "                    \
                        << #replacement_classname << " instead."              \
                        << std::endl);                                        \
    ::IMP::base::internal::set_printed_deprecation_message(#classname, true); \
  }
/** \deprecated_at{2.1} As of IMP 2.1. Use IMPMODULE_DEPRECATED_FUNCTION_DEF()
    instead
 */
#define IMP_DEPRECATED_FUNCTION(replacement)                                \
  IMP_DEPRECATED_MACRO(2.1,                                                 \
                   "Use IMP@MODULE@_DEPRECATED_FUNCTION_DECL/DEF instead"); \
  if (::IMP::base::internal::get_print_deprecation_message(                 \
          IMP_CURRENT_FUNCTION)) {                                          \
    IMP_WARN(IMP_CURRENT_FUNCTION << " is deprecated "                      \
                                  << "and should not be used.\nUse "        \
                                  << #replacement << " instead."            \
                                  << std::endl);                            \
    ::IMP::base::internal::set_printed_deprecation_message(                 \
        IMP_CURRENT_FUNCTION, true);                                        \
  }

#if !defined(IMP_SWIG_WRAPPER) && (defined(__GNUC__) || defined(__clang__))
#define IMP_DEPRECATED_WARN                                          \
  IMP_DEPRECATED_MACRO(2.1, "Use IMPMODULE_DEPRECATED_FUNCTION_DEF") \
      __attribute__((deprecated))
#else
/** Produce compiler warnings when the function is called.

    \deprecated_at{2.1} As of IMP 2.1. Use IMPMODULE_DEPRECATED_FUNCTION_DEF()
    instead.
*/
#define IMP_DEPRECATED_WARN
#endif

#define IMP_DEPRECATED_FUNCTION_DEF(version, message) \

#endif /* IMPBASE_DEPRECATION_MACROS_H */
