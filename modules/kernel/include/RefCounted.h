/**
 *  \file IMP/kernel/RefCounted.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_REF_COUNTED_H
#define IMPKERNEL_REF_COUNTED_H

#include <IMP/kernel/kernel_config.h>
// skip deprecated header in kernel.h
#ifndef IMPKERNEL_ALL
#include <IMP/base/RefCounted.h>

IMPKERNEL_DEPRECATED_HEADER(2.1, "Use base/RefCounted.h");

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(IMP_DOXYGEN)
using IMP::base::RefCounted;
#endif

IMPKERNEL_END_NAMESPACE
#endif

#endif /* IMPKERNEL_REF_COUNTED_H */
