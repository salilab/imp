/**
 *  \file constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_CONSTANTS_H
#define IMP_CONSTANTS_H

#include "config.h"
#include <cmath>

IMP_BEGIN_NAMESPACE

//! Avagadro's number
static const double NA=6.02214179e-23;


#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
//! the constant pi
static const double PI= M_PI;
#else
//! the constant pi
static const double PI = 3.1415926535897931;
#endif // C99

IMP_END_NAMESPACE

#endif  /* IMP_CONSTANTS_H */
