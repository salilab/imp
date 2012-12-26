/**
 *  \file IMP/base/warning_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_WARNING_MACROS_H
#define IMPBASE_WARNING_MACROS_H

#include "base_config.h"
#include "internal/eat.h"

#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
/*
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)                       \
  IMP_NO_DOXYGEN(IMP_NO_SWIG(struct semicolon_##Name##_helper_struct{}))
#define IMP_REQUIRE_SEMICOLON_NAMESPACE void dummy_f()
*/


/** Disable unused variable warning for a variable.
 */
#define IMP_UNUSED(variable) IMP::base::internal::eat(variable)

#if IMP_BUILD != IMP_FAST
/** Mark a variable as one that is only used in checks. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_CHECK_VARIABLE(variable)

/** Mark a variable as one that is only used in logging. This disables
    unused variable warnings on it in fast mode.
*/
#define IMP_LOG_VARIABLE(variable)

#else
#define IMP_CHECK_VARIABLE(variable) IMP_UNUSED(variable)
#define IMP_LOG_VARIABLE(variable) IMP_UNUSED(variable)

#endif

#ifndef IMP_DOXYGEN
#ifdef __GNUC__
#define IMP_WARN_PREPROCESS(msg) IMP_PRAGMA(message #msg)

//#if __GNUC_PREREQ(4,2)
#define IMP_GCC_DISABLE_WARNING(name)\
_Pragma(IMP_STRINGIFY(GCC diagnostic ignored name))

/*#else
#define IMP_GCC_DISABLE_WARNING(name)
#endif*/

#else
#define IMP_GCC_DISABLE_WARNING(name)
#endif

#endif


#endif  /* IMPBASE_WARNING_MACROS_H */
