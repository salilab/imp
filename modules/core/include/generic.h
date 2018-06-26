/**
 *  \file IMP/core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GENERIC_H
#define IMPCORE_GENERIC_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>

IMPKERNEL_BEGIN_NAMESPACE
/** A Python version of this is provided, but it produces a slightly less
    efficient restraint. */
using IMP::create_restraint;
using IMP::create_constraint;

IMPKERNEL_END_NAMESPACE

#endif /* IMPCORE_GENERIC_H */
