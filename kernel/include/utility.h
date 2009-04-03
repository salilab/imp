/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_UTILITY_H
#define IMP_UTILITY_H

#include "macros.h"
#include "config.h"

#include <boost/version.hpp>
#include <algorithm>

#if BOOST_VERSION >= 103500
#include <boost/math/special_functions/fpclassify.hpp>
#else
#ifdef __GNUC__
#include <cmath>
#endif // __GNUC__
#endif // BOOST_VERSION

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
template <class T>
inline bool is_nan(const T& a) {
#if BOOST_VERSION >= 103500
  return (boost::math::isnan)(a);
#else
  // Not all gcc versions include C99 math
#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
  return (std::isnan)(a);
#else
  return a != a;
#endif // C99
#endif // BOOST_VERSION
}



//! A version of std::for_each which works with ranges
/** This is needed to apply the functor to a range which is a temporary
    object, since you can't call both begin and end on it.
 */
template <class Range, class Functor>
void for_each(const Range &r, const Functor &f) {
  std::for_each(r.begin(), r.end(), f);
}
IMP_END_NAMESPACE

#endif  /* IMP_UTILITY_H */
