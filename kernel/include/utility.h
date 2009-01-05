/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_UTILITY_H
#define IMP_UTILITY_H

#include "macros.h"

#ifdef __GNUC__
#include <cmath>
#endif


IMP_BEGIN_NAMESPACE

//! Compute the square of a number
template <class T>
T square(T t)
{
  return t*t;
}


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

IMP_END_NAMESPACE

#endif  /* IMP_UTILITY_H */
