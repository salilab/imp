/**
 *  \file IMP/base_utility.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_BASE_UTILITY_H
#define IMPKERNEL_BASE_UTILITY_H

#include <IMP/kernel_config.h>
#include <boost/utility.hpp>
#include <cmath>
#include "base_macros.h"
#include <IMP/Vector.h>
#include <IMP/math.h>

IMPKERNEL_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T>
inline T square(T t) IMP_NO_SIDEEFFECTS;
template <class T>
inline T cube(T t) IMP_NO_SIDEEFFECTS;

//! Compute the square of a number
template <class T>
inline T square(T t) {
  return t * t;
}

//! Compute the cube of a number
template <class T>
inline T cube(T t) {
  return t * t * t;
}

template <class T>
inline bool is_nan(const T &a) {
  return isnan(a);
}

//! A version of std::for_each which works with ranges
/** This is needed to apply the functor to a range which is a temporary
    object, since you can't call both begin and end on it.
 */
template <class Range, class Functor>
inline void for_each(const Range &r, const Functor &f) {
  std::for_each(r.begin(), r.end(), f);
}

template <class T>
inline int compare(const T &a, const T &b) {
  return a.compare(b);
}

//! Convert between different types of lists.
template <class Out, class In>
inline Out get_as(const In &in) {
  return Out(in.begin(), in.end());
}
#endif

//! Return a unique name produced from the string.
/** This is done by replacing %1% with a sequential number.*/
IMPKERNELEXPORT std::string get_unique_name(std::string templ);

//! Get the IMP copyright notice
IMPKERNELEXPORT std::string get_copyright();

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_BASE_UTILITY_H */
