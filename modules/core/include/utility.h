/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "macros.h"
#include "config.h"
#include <IMP/base_types.h>

#ifdef __GNUC__
#include <cmath>
#endif

IMPCORE_BEGIN_NAMESPACE

//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
 out a!=a (and certain intel chips had issues with it too).
 */
inline bool is_nan(const float& a) {
  // Not all gcc versions include C99 math
#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
  return std::isnan(a);
#else
  return a != a;
#endif
}

//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
 out a!=a (and certain intel chips had issues with it too).
 */
inline bool is_nan(const double& a) {
#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
  return std::isnan(a);
#else
  return a != a;
#endif
}

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_UTILITY_H */
