/**
 *  \file IMP/base/deprecation.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_DEPRECATION_H
#define IMPBASE_DEPRECATION_H

#include <IMP/base/base_config.h>

IMPBASE_BEGIN_NAMESPACE

//! Toggle printing of warnings on using deprecated classes
/** If set to true (the default) a warning is printed every
    time a class marked as deprecated is used.
 */
IMPBASEEXPORT void set_deprecation_warnings(bool tf);

/** Toggle whether an exception is thrown when a deprecated
    method is used.
 */
IMPBASEEXPORT void set_deprecation_exceptions(bool tf);

/** Break in this method in gdb to find deprecated uses at runtime. */
IMPBASEEXPORT void handle_use_deprecated(std::string message);

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_DEPRECATION_H */
