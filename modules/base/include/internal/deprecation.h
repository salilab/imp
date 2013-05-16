/**
 *  \file internal/deprecation.h
 *  \brief Macros to mark a class as deprecated.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_INTERNAL_DEPRECATION_H
#define IMPBASE_INTERNAL_DEPRECATION_H

#include <IMP/base/base_config.h>

IMPBASE_BEGIN_INTERNAL_NAMESPACE
IMPBASEEXPORT bool get_print_deprecation_message(std::string name);

IMPBASEEXPORT void set_printed_deprecation_message(std::string name, bool tr);

IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_DEPRECATION_H */
