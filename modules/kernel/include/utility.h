/**
 *  \file IMP/utility.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UTILITY_H
#define IMPKERNEL_UTILITY_H

#include <IMP/kernel_config.h>
#include <IMP/utility.h>
#include <IMP/types.h>
#include <IMP/Vector.h>
IMPKERNEL_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
using IMP::cube;
using IMP::square;
using IMP::is_nan;
using IMP::get_as;
#endif
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_UTILITY_H */
