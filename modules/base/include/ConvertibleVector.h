/**
 *  \file ConvertibleVector.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CONVERTIBLE_VECTOR_H
#define IMPBASE_CONVERTIBLE_VECTOR_H
#include "base_config.h"
#include <IMP/compatibility/vector.h>

namespace IMP {
#ifndef SWIG
/* MSVC gets very confused (error C2872) between std::vector and
   IMP::compatibility::vector if we include headers (e.g. OpenCV) that use
   a plain 'vector' after 'using namespace std'. Since compatibility::vector
   doesn't add anything to std::vector anyway on MSVC, use std::vector instead
   here. */
# ifdef _MSC_VER
  using std::vector;
# else
  using compatibility::vector;
# endif
#else
  template <class T>
  struct vector {};
#endif
}


IMPBASE_BEGIN_NAMESPACE

template <class T>
class ConvertibleVector: public IMP::vector<T> {
  typedef IMP::vector<T> V;
 public:
  ConvertibleVector(){}
  ConvertibleVector(unsigned int sz, const T&t=T()): V(sz, t){}
  template <class It>
  ConvertibleVector(It b, It e): V(b,e){}
  template <class O>
  ConvertibleVector(const IMP::vector<O> &o): V(o.begin(),
                                                      o.end()){}
   template <class O>
  operator IMP::vector<O>() const {
    return IMP::vector<O>(V::begin(), V::end());
  }
};
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONVERTIBLE_VECTOR_H */
