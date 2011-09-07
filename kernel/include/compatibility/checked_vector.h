/**
 *  \file IMP/compatibility/checked_vector.h
 *  \brief Declare a bounds checked vector
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMP_COMPATIBILITY_CHECKED_VECTOR_H
#define IMP_COMPATIBILITY_CHECKED_VECTOR_H

#include "compatibility_config.h"

#if IMP_BUILD < IMP_RELEASE && defined(__GNUC__)
#include <debug/vector>
#define IMP_VECTOR_PARENT __gnu_debug::vector
#else
#include <vector>
#define IMP_VECTOR_PARENT std::vector
#endif


IMP_BEGIN_COMPATIBILITY_NAMESPACE
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
class checked_vector: public IMP_VECTOR_PARENT<T>
{
public:
  template <class It>
  checked_vector(It b, It e): IMP_VECTOR_PARENT<T>(b,e){}
  checked_vector(unsigned int ct, const T& t=T()):
    IMP_VECTOR_PARENT<T>(ct, t){}
  checked_vector(){}
};

template <class T>
void swap(checked_vector<T> &a,
          checked_vector<T> &b) {
  std::swap(static_cast<IMP_VECTOR_PARENT<T> &>(a),
    static_cast<IMP_VECTOR_PARENT<T> &>(b));
}
#endif

IMP_END_COMPATIBILITY_NAMESPACE

#endif  /* IMP_COMPATIBILITY_CHECKED_VECTOR_H */
