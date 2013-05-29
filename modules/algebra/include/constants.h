/**
 *  \file IMP/algebra/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_CONSTANTS_H
#define IMPALGEBRA_CONSTANTS_H

#include <IMP/algebra/algebra_config.h>
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE

#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
//! the constant pi
static const double PI = M_PI;
#else
//! the constant pi
static const double PI = 3.1415926535897931;
#endif  // C99

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CONSTANTS_H */
