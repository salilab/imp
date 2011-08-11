/**
 *  \file IMP/compatibility/checked_vector.h
 *  \brief Declare a bounds checked vector
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMP_COMPATIBILITY_CHECKED_VECTOR_H
#define IMP_COMPATIBILITY_CHECKED_VECTOR_H

#include "compatibility_config.h"

#if IMP_BUILD < IMP_FAST && defined(__GNUC__)
#define IMP_USE_CHECKED_STL
#include <debug/vector>
#else
#include <vector>
#endif


IMP_BEGIN_COMPATIBILITY_NAMESPACE
#ifdef SWIG
template <class T>
class checked_vector{};
#elif defined(IMP_DOXYGEN)
/** This class defines a std::vector-like template that, when available
    and IMP is not built in fast mode, performs bounds checking. In general
    IMP code should prefer this vector to std::vector for internal storage.

    Currently, bounds checking is provided when compiling with gcc.
*/
template <class T>
class checked_vector{};
#elif defined(IMP_USE_CHECKED_STL)

template <class T>
class checked_vector: public __gnu_debug::vector<T>
{
  typedef __gnu_debug::vector<T> P;
public:
  template <class It>
  checked_vector(It b, It e): P(b,e){}
  checked_vector(unsigned int ct, const T& t=T()): P(ct, t){}
  checked_vector(){}
};

template <class T>
void swap(checked_vector<T> &a,
          checked_vector<T> &b) {
  std::swap(static_cast<std::vector<T> &>(a),
            static_cast<std::vector<T> &>(b));
}
#else

template <class T>
class checked_vector: public std::vector<T>
{
  typedef std::vector<T> P;
public:
  template <class It>
  checked_vector(It b, It e): P(b,e){}
  checked_vector(unsigned int ct, const T& t=T()): P(ct, t){}
  checked_vector(){}
};

template <class T>
void swap(checked_vector<T> &a,
          checked_vector<T> &b) {
  std::swap(static_cast<std::vector<T> &>(a),
            static_cast<std::vector<T> &>(b));
}
#endif

IMP_END_COMPATIBILITY_NAMESPACE

#endif  /* IMP_COMPATIBILITY_CHECKED_VECTOR_H */
