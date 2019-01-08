/**
 *  \file IMP/deprecation_macros.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DEPRECATION_MACROS_H
#define IMPKERNEL_DEPRECATION_MACROS_H

#include <IMP/kernel_config.h>
#include "deprecation.h"

#if !defined(SWIG)

/** Used to implement deprecation support. See the
    [IMP deprecation policy](@ref deprecation).*/
#define IMP_DEPRECATED_MACRO(version, help_message)                 \
  IMP_PRAGMA(message("This macro is deprecated as of IMP " #version \
                     ": " help_message))

/** Used to implement deprecation support. See the
    [IMP deprecation policy](@ref deprecation).*/
#define IMP_DEPRECATED_OBJECT_RUNTIME_WARNING(version, help_message) \
  {                                                                  \
    std::ostringstream oss;                                          \
    oss << "Object " << get_module_name() << "::" << get_type_name() \
        << " is deprecated. " << help_message << std::endl;          \
    IMP::handle_use_deprecated(oss.str());                     \
  }

/** Used to implement deprecation support. See the
    [IMP deprecation policy](@ref deprecation).*/
#define IMP_DEPRECATED_VALUE_RUNTIME_WARNING(version, help_message)      \
  {                                                                      \
    std::ostringstream oss;                                              \
    oss << "Class " << get_module_name() << "::" << IMP_CURRENT_FUNCTION \
        << " is deprecated. " << help_message << std::endl;              \
    IMP::handle_use_deprecated(oss.str());                         \
  }

/** Used to implement deprecation support. See the
    [IMP deprecation policy](@ref deprecation).*/
#define IMP_DEPRECATED_FUNCTION_RUNTIME_WARNING(version, help_message)      \
  {                                                                         \
    std::ostringstream oss;                                                 \
    oss << "Function " << IMP_CURRENT_PRETTY_FUNCTION << " is deprecated. " \
        << help_message << std::endl;                                       \
    IMP::handle_use_deprecated(oss.str());                            \
  }

/** Used to implement deprecation support. See the
    [IMP deprecation policy](@ref deprecation).*/
#define IMP_DEPRECATED_METHOD_RUNTIME_WARNING(version, help_message)      \
  {                                                                       \
    std::ostringstream oss;                                               \
    oss << "Method " << IMP_CURRENT_PRETTY_FUNCTION << " is deprecated. " \
        << "WARNING: " << help_message << std::endl;                      \
    IMP::handle_use_deprecated(oss.str());                          \
  }

#define IMP_DEPRECATED_FUNCTION_DEF(version, message)

#endif // SWIG

#endif /* IMPKERNEL_DEPRECATION_MACROS_H */
