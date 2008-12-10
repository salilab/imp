/**
 *  \file internal/deprecation.h
 *  \brief Macros to mark a class as deprecated.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_DEPRECATION_H
#define IMPCORE_INTERNAL_DEPRECATION_H

#include "../config.h"
#include "../macros.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

IMPCOREEXPORT bool get_print_deprecation_message(const char *name);

IMPCOREEXPORT void set_printed_deprecation_message(const char *name, bool tr);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_DEPRECATION_H */
