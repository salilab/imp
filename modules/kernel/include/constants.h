/**
 *  \file IMP/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONSTANTS_H
#define IMPKERNEL_CONSTANTS_H

#include <IMP/kernel_config.h>
#include <cmath>

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
//! the constant pi
static const double PI = M_PI;
#else
//! the constant pi
static const double PI = 3.1415926535897931;
#endif  // C99
#endif

//! Use this value when you want to turn off maximum for restraint evaluation
IMPKERNELEXPORT extern const double NO_MAX;
/** Evaluation can return this value if limits are exceeded and
    it is a ScoringFunction::evaluate_if_below() or
    ScoringFunction::evaluate_if_good
*/
IMPKERNELEXPORT extern const double BAD_SCORE;
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONSTANTS_H */
