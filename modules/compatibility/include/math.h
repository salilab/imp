/**
 *  \file IMP/compatibility/math.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_COMPATIBILITY_MATH_H
#define IMPCOMPATIBILITY_COMPATIBILITY_MATH_H

#include <IMP/compatibility/compatibility_config.h>
#include <boost/version.hpp>
#include <cmath>
#if !defined(_GLIBCXX_USE_C99_MATH)
#include <boost/math/special_functions/fpclassify.hpp>
#endif

IMPCOMPATIBILITY_BEGIN_NAMESPACE
//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
 out a!=a (and certain intel chips had issues with it too).
 */
template <class T>
inline bool isnan(const T& a) {
#if defined(_GLIBCXX_USE_C99_MATH)
  // Not all gcc versions include C99 math
  return (std::isnan)(a);
#else
  return (boost::math::isnan)(a);
#endif
}

//! Return true if a number is infinite
template <class T>
inline bool isinf(const T& a) {
#if defined(_GLIBCXX_USE_C99_MATH)
  // Not all gcc versions include C99 math
  return (std::isinf)(a);
#else
  return (boost::math::isinf)(a);
#endif
}

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_COMPATIBILITY_MATH_H */
