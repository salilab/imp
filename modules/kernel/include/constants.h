/**
 *  \file IMP/kernel/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONSTANTS_H
#define IMPKERNEL_CONSTANTS_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/algebra/constants.h>
#include <cmath>

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
static const double PI= algebra::PI;
#endif

//! Use this value when you want to turn off maximum for restraint evaluation
IMPKERNELEXPORT extern const double NO_MAX;
/** Evaluation can return this value if limits are exceeded and
    it is a ScoringFunction::evaluate_if_below() or
    ScoringFunction::evaluate_if_good
*/
IMPKERNELEXPORT extern const double BAD_SCORE;
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_CONSTANTS_H */
