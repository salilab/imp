/**
 *  \file internal/deprecation.h
 *  \brief Macros to mark a class as deprecated.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_DEPRECATION_H
#define IMP_INTERNAL_DEPRECATION_H

#include "../config.h"

IMP_BEGIN_INTERNAL_NAMESPACE

IMPEXPORT bool get_print_deprecation_message(const char *name);

IMPEXPORT void set_printed_deprecation_message(const char *name, bool tr);

IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_DEPRECATION_H */
