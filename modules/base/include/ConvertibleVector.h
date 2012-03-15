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

IMPBASE_BEGIN_NAMESPACE

template <class T>
class ConvertibleVector: public compatibility::vector<T> {
  typedef compatibility::vector<T> V;
 public:
  ConvertibleVector(){}
  ConvertibleVector(unsigned int sz, const T&t=T()): V(sz, t){}
  template <class It>
  ConvertibleVector(It b, It e): V(b,e){}
  template <class O>
  ConvertibleVector(const compatibility::vector<O> &o): V(o.begin(),
                                                      o.end()){}
   template <class O>
  operator compatibility::vector<O>() const {
    return compatibility::vector<O>(V::begin(), V::end());
  }
};
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONVERTIBLE_VECTOR_H */
