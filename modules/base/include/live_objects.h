/**
 *  \file IMP/base/live_objects.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_LIVE_OBJECTS_H
#define IMPBASE_LIVE_OBJECTS_H

#include <IMP/base/base_config.h>
#include "Object.h"
#include "types.h"

IMPBASE_BEGIN_NAMESPACE
#if IMP_HAS_CHECKS
/** Return the names of all live objects, use this to check for memory
    leaks.*/
IMPBASEEXPORT Strings get_live_object_names();
/** Return the points to all live objects, use this to check for memory
    leaks.*/
IMPBASEEXPORT Objects get_live_objects();

/** Set whether to complain about objects not being properly cleaned up.
 */
IMPBASEEXPORT void set_show_leaked_objects(bool tf);
#endif


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_LIVE_OBJECTS_H */
