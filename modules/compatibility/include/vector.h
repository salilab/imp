/**
 *  \file IMP/compatibility/vector.h
 *  \brief Declare a bounds checked vector
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_VECTOR_H
#define IMPCOMPATIBILITY_VECTOR_H

#include "compatibility_config.h"

#if IMP_BUILD < IMP_RELEASE && defined(__GNUC__)
#include <debug/vector>
#define IMPCOMPATIBILITY_VECTOR_PARENT __gnu_debug::vector
#else
#include <vector>
#define IMPCOMPATIBILITY_VECTOR_PARENT std::vector
#endif


IMPCOMPATIBILITY_BEGIN_NAMESPACE
#ifdef SWIG
template <class T>
class vector: public std::vector<T> {};

#elif defined(IMP_DOXYGEN)
/** This class defines a std::vector-like template that, when available
    and IMP is not built in fast mode, performs bounds checking. In general
    IMP code should prefer this vector to std::vector for internal storage.

    Currently, bounds checking is provided when compiling with gcc.
*/
typedef std::vector vector;

#else

template <class T>
class vector: public IMPCOMPATIBILITY_VECTOR_PARENT<T>
{
public:
  template <class It>
  vector(It b, It e): IMPCOMPATIBILITY_VECTOR_PARENT<T>(b,e){}
  explicit vector(unsigned int ct, const T& t=T()):
    IMPCOMPATIBILITY_VECTOR_PARENT<T>(ct, t){}
  vector(){}
};

template <class T>
void swap(vector<T> &a,
          vector<T> &b) {
  std::swap(static_cast<IMPCOMPATIBILITY_VECTOR_PARENT<T> &>(a),
    static_cast<IMPCOMPATIBILITY_VECTOR_PARENT<T> &>(b));
}
#endif

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_VECTOR_H */
