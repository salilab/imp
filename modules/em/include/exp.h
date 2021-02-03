/**
 *  \file IMP/em/exp.h
 *  \brief An approximation of the exponential function.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_EXP_H
#define IMPEM_EXP_H

#include <IMP/em/em_config.h>

/* An approximation of the exponential function.
   Schraudolph, Nicol N. "A Fast, Compact Approximation of the exponential
   function" Technical Report IDSIA-07-98
   (Neural Computation 11(4))
   The function returns values within 1-2% deviation of the correct ones in
   the range (-700,700)
*/

#include <IMP/em/em_config.h>
#include <math.h>
#include <boost/version.hpp>

#if BOOST_VERSION < 106900
#include <boost/detail/endian.hpp>
#else
#include <boost/predef/other/endian.h>
#endif

IMPEM_BEGIN_NAMESPACE

/* Unions are similar to structs, but they differ in one aspect:
   the fields of a union share the same position in memory.
  The size of the union is the size of its largest field
  (or larger if alignment so requires, for example on a SPARC machine
  a union contains a double and a char [17] so its size is likely to be
  24 because it needs 64-bit alignment).
  What is the point of this? Unions provide multiple ways of viewing
  the same memory location, allowing for \more efficient use of memory.
  Most of the uses of unions are covered by object-oriented features of C++,
  so it is more common in C. However, sometimes it is convenient to avoid
  the formalities of object-oriented programming when performance is
  important or when one knows that the item in question will not be extended.
  http://en.wikibooks.org/wiki/C++_Programming/Union
*/
typedef union {
  double d;
  struct {
#if defined(BOOST_ENDIAN_LITTLE_BYTE) || defined(BOOST_LITTLE_ENDIAN)
    int j, i;
#else
    int i, j;
#endif
  } n;
} _eco;

/* Originally was (1048576/M_LN2); M_LN2 expanded out so that this header
   can be correctly #include'd in other code on Windows machines without having
   to #define the non-standard _USE_MATH_DEFINES macro */
#define IMP_EXP_A (1048576 / 0.693147180559945309417232121458176568)
#define IMP_EXP_C 60801

inline double EXP(float y) {
  static _eco eco;
  eco.n.i = (int)(IMP_EXP_A * (y) + (1072693248 - IMP_EXP_C));
  return eco.d;
}

IMPEM_END_NAMESPACE

#endif /* IMPEM_EXP_H */
