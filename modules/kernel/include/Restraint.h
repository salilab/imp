/**
 *  \file IMP/Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_H
#define IMPKERNEL_RESTRAINT_H

#include "kernel_config.h"
#include "declare_Restraint.h"

IMP_BEGIN_NAMESPACE

/** Return the decomposition of a list of restraints. */
IMPEXPORT Restraints create_decomposition(const RestraintsTemp &rs);

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_RESTRAINT_H */
