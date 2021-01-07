/**
 *  \file IMP/deprecation.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DEPRECATION_H
#define IMPKERNEL_DEPRECATION_H

#include <IMP/kernel_config.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Toggle printing of warnings on using deprecated classes
/** If set to true (the default) a warning is printed every
    time a class marked as deprecated is used.
 */
IMPKERNELEXPORT void set_deprecation_warnings(bool tf);

//! Toggle whether an exception is thrown when a deprecated method is used.
IMPKERNELEXPORT void set_deprecation_exceptions(bool tf);

//! Get whether an exception is thrown when a deprecated method is used.
IMPKERNELEXPORT bool get_deprecation_exceptions();

/** Break in this method in gdb to find deprecated uses at runtime. */
IMPKERNELEXPORT void handle_use_deprecated(std::string message);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_DEPRECATION_H */
