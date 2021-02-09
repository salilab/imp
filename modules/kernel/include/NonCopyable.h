/**
 *  \file IMP/NonCopyable.h
 *  \brief Base class for all objects that cannot be copied.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_NON_COPYABLE_H
#define IMPKERNEL_NON_COPYABLE_H

#include <IMP/kernel_config.h>
#include "utility_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for all objects that cannot be copied.
/** Non-copyable classes cannot be copied, as the name implies. If you need
    to do delayed initialization, either use an initialize method on them or
    use a boost::scoped_ptr to allocate them on the heap.
  */
class NonCopyable {
 protected:
  NonCopyable() {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_NON_COPYABLE_H */
