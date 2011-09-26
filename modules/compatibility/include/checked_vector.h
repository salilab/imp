/**
 *  \file IMP/compatibility/checked_vector.h
 *  \brief Declare a bounds checked vector
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_CHECKED_VECTOR_H
#define IMPCOMPATIBILITY_CHECKED_VECTOR_H

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
class checked_vector: public std::vector<T> {};

#elif defined(IMP_DOXYGEN)
/** This class defines a std::vector-like template that, when available
    and IMP is not built in fast mode, performs bounds checking. In general
    IMP code should prefer this vector to std::vector for internal storage.

    Currently, bounds checking is provided when compiling with gcc.
*/
typedef std::vector checked_vector;

#else

template <class T>
class checked_vector: public IMPCOMPATIBILITY_VECTOR_PARENT<T>
{
public:
  template <class It>
  checked_vector(It b, It e): IMPCOMPATIBILITY_VECTOR_PARENT<T>(b,e){}
  checked_vector(unsigned int ct, const T& t=T()):
    IMPCOMPATIBILITY_VECTOR_PARENT<T>(ct, t){}
  checked_vector(){}
};

template <class T>
void swap(checked_vector<T> &a,
          checked_vector<T> &b) {
  std::swap(static_cast<IMPCOMPATIBILITY_VECTOR_PARENT<T> &>(a),
    static_cast<IMPCOMPATIBILITY_VECTOR_PARENT<T> &>(b));
}
#endif

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_CHECKED_VECTOR_H */
