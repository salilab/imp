/**
 *  \file internal/deprecation.h
 *  \brief Macros to mark a class as deprecated.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_DEPRECATION_H
#define IMPKERNEL_INTERNAL_DEPRECATION_H

#include <IMP/kernel_config.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
IMPKERNELEXPORT bool get_print_deprecation_message(std::string name);

IMPKERNELEXPORT void set_printed_deprecation_message(std::string name, bool tr);

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_DEPRECATION_H */
