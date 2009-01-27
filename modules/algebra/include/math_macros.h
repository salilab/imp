/**
 *  \file math_macros.h
 *  \brief Macros to deal with very common math operations
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_MATH_MACROS_H
#define IMPALGEBRA_MATH_MACROS_H

IMP_BEGIN_NAMESPACE

//! Simple interpolation that is only valid for values of a ranging from 0 to 1.
/**
 *
 * \return The returned value is the low (l) argument when a=0 and the high
 * (h) argument when a=1. Specifically, l+(h-l)*a
 */
#define SIMPLE_INTERP(a, l, h) ((l) + ((h) - (l)) * (a))

//! xor operation between two values (non necessarily bites)
template<typename T1, typename T2>
bool xorT(const T1& x, const T2& y)
{
  return (((x) && !(y)) || (!(x) && (y)));
}

//! Sign of a number. 1 if the number is higher or equal to 0 and -1 otherwise
template<typename T>
int sgn(const T& x)
{
  if (x >= 0) return 1;
  return -1;
}

//! Rounds a number to next integer.
/**
 * The result is of type integer but the argument can be of any type. Some
 * examples:
 *
 * \code
 * a = round(-0.8); // a = -1
 * a = round(-0.2); // a = 0
 * a = round(0.2); // a = 0
 * a = round(0.8); // a = 1
 * \endcode
 */
template<typename T>
int round(const T& x)
{
  if (x > 0) {
    return (int)((x) + 0.5);
  } else {
    return (int)((x) - 0.5);
  }
}


//! Thresholds a value between two given limits
/*
 * The limits act as the thresholds to saturate the value. Examples:
 * with x and threshold(x,-2,2):
 *
 * \code
 * x = threshold(-8,-2,2); // x=-2;
 * x = threshold(-2,-2,2); // x=-2;
 * x = threshold(-1,-2,2); // x=-1;
 * x = threshold(0,-2,2); // x=0;
 * x = threshold(2,-2,2); // x=2;
 * x = threshold(4,-2,2); // x=2;
 * \endcode
 */
template<typename T>
T threshold(const T x, const T x0, const T xF)
{
  if (x < x0) return x0;
  if (x > xF) return xF;
  return x;
}


//! Compares two values (intended for float or doubles)
/**
 * epsilon is the tolerance allowed to consider the values as equal
 */
template<typename T>
bool equal(const T a, const T b, const T epsilon)
{
  return (std::abs(a - b) < epsilon);
}

IMP_END_NAMESPACE

#endif  /* IMPALGEBRA_MATH_MACROS_H */
