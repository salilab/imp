/**
 *  \file IMP/base/NonCopyable.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_NON_COPYABLE_H
#define IMPBASE_NON_COPYABLE_H

#include <IMP/base/base_config.h>
#include "utility_macros.h"

IMPBASE_BEGIN_NAMESPACE
/** Non-copyable classes cannot be copied, as the name implies. If you need
    to do delayed initialization, either use an initialize method on them or
    use a boost::scoped_ptr to allocate them on the heap.
*/
class NonCopyable {
 protected:
  NonCopyable(){}
};
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_NON_COPYABLE_H */
