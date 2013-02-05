/**
 *  \file IMP/kernel/WeakPointer.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_WEAK_POINTER_H
#define IMPKERNEL_WEAK_POINTER_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/WeakPointer.h>


IMPKERNEL_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
using IMP::base::UncheckedWeakPointer;
using IMP::base::WeakPointer;
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_WEAK_POINTER_H */
