/**
 *  \file IMP/kernel/utility.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UTILITY_H
#define IMPKERNEL_UTILITY_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/utility.h>
#include <IMP/base/types.h>
#include <IMP/base/Vector.h>
IMPKERNEL_BEGIN_NAMESPACE
#ifndef SWIG
#ifndef IMP_DOXYGEN
using IMP::base::cube;
using IMP::base::square;
using IMP::base::is_nan;
#endif
using IMP::base::get_as;
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_UTILITY_H */
