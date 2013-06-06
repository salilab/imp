/**
 *  \file IMP/kernel/Pointer.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_POINTER_H
#define IMPKERNEL_POINTER_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_NAMESPACE

IMPKERNEL_DEPRECATED_HEADER(2.1, "Please use IMP/base/Pointer.h instead.");

#ifndef IMP_DOXYGEN
using IMP::base::Pointer;
using IMP::base::OwnerPointer;
#endif
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_POINTER_H */
