/**
 *  \file IMP/live_objects.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_LIVE_OBJECTS_H
#define IMPKERNEL_LIVE_OBJECTS_H

#include <IMP/kernel_config.h>
#include "Object.h"
#include "types.h"

IMPKERNEL_BEGIN_NAMESPACE
#if IMP_HAS_CHECKS
//! Return the names of all live objects.
/** Use this to check for memory leaks. */
IMPKERNELEXPORT Strings get_live_object_names();

//! Return pointers to all live objects.
/** Use this to check for memory leaks. */
IMPKERNELEXPORT Objects get_live_objects();

//! Set whether to complain about objects not being properly cleaned up.
IMPKERNELEXPORT void set_show_leaked_objects(bool tf);
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_LIVE_OBJECTS_H */
