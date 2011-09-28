/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UTILITY_H
#define IMPKERNEL_UTILITY_H

#include "kernel_config.h"
#include <IMP/base/utility.h>

IMP_BEGIN_NAMESPACE
#ifndef SWIG
using IMP::base::cube;
using IMP::base::square;
using IMP::base::operator<<;
using IMP::base::is_nan;
using IMP::base::get_as;
#endif
IMP_END_NAMESPACE

#endif  /* IMPKERNEL_UTILITY_H */
