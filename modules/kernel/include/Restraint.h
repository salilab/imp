/**
 *  \file IMP/kernel/Restraint.h
    \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_H
#define IMPKERNEL_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>
#include "declare_Restraint.h"
// needed due to Tracker stuff
#include "Model.h"

IMPKERNEL_BEGIN_NAMESPACE

/** Return the decomposition of a list of restraints. */
IMPKERNELEXPORT Restraints create_decomposition(const RestraintsTemp &rs);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_RESTRAINT_H */
