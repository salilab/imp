/**
 *  \file IMP/algebra/utility.h
 *  \brief Functions to deal with very common math operations
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_UTILITY_H
#define IMPALGEBRA_UTILITY_H

#include <cmath>  // abs
#include <IMP/algebra/algebra_config.h>
#include <IMP/base/exception.h>

IMPALGEBRA_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
//! xor operation between two values
inline bool xorT(bool x, bool y) { return (((x) && !(y)) || (!(x) && (y))); }
#endif

inline double get_squared(double x) { return x * x; }

//! Sign of a number. 1 if the number is higher or equal to 0 and -1 otherwise
template <typename T>
inline int get_sign(const T& x) {
  if (x >= 0) return 1;
  return -1;
}

//! Rounds a number to next integer.
/**
 * The result is of type integer but the argument can be of any type. Some
 * examples:
 *
 * \code
 * a = round(-0.7); // a = -1
 * a = round(-0.2); // a = 0
 * a = round(0.2); // a = 0
 * a = round(0.7); // a = 1
 * \endcode
 */
template <typename T>
inline int get_rounded(const T& x) {
  if (x > 0) {
    return static_cast<int>((x) + 0.5);
  } else {
    return static_cast<int>((x) - 0.5);
  }
}

//! Constrains a value between two given limits
/*
 * The limits act as the constrain to saturate the value. Examples:
 * with x and constrain(x,-2,2):
 *
 * \code
 * x = constrain(-8,-2,2); // x=-2;
 * x = constrain(-2,-2,2); // x=-2;
 * x = constrain(-1,-2,2); // x=-1;
 * x = constrain(0,-2,2); // x=0;
 * x = constrain(2,-2,2); // x=2;
 * x = constrain(4,-2,2); // x=2;
 * \endcode
 */
template <typename T>
inline T get_constrained(const T x, const T x0, const T xF) {
  if (x < x0) return x0;
  if (x > xF) return xF;
  return x;
}

//! Closest power of 2 that can contain a number x
inline float get_next_larger_power_of_2(float x) {
  float p = 1;
  while (p < x) {
    p *= 2;
  }
  return p;
}

//! Closest power of 2 that can contain a number x
inline double get_next_larger_power_of_2(double x) {
  double p = 1;
  while (p < x) {
    p *= 2;
  }
  return p;
}

//! Closest power of 2 for a number, not necessarily higher
inline double get_closer_power_of_2(double x) {
  double p = 1;
  double q = 1;
  while (p < x) {
    q = p;
    p *= 2;
  }
  if ((x - q) < (p - x)) {
    return q;
  }
  return p;
}

//! Compares two values (intended for doubles)
/**
 * epsilon is the tolerance allowed to consider the values as equal
 */
inline bool get_are_almost_equal(const double a, const double b,
                                 const double epsilon) {
  return (std::abs(a - b) < epsilon);
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_UTILITY_H */
