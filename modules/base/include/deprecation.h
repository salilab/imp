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
#include "internal/deprecation.h"

IMPBASE_BEGIN_NAMESPACE

//! Toggle printing of warnings on using deprecated classes
/** If set to true (the default) a warning is printed every
    time a class marked as deprecated is used.
    \sa IMP_DEPRECATED
 */
IMPBASEEXPORT void set_print_deprecation_messages(bool tf);

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_DEPRECATION_H */
