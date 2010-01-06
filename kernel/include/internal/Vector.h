/**
 *  \file Vector.h    \brief A bounds checked vector.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_VECTOR_H
#define IMP_VECTOR_H

#include "../exception.h"
#include "../Object.h"
#include "../RefCounted.h"
#include "ref_counting.h"

#include <vector>
#include <algorithm>

IMP_BEGIN_NAMESPACE

class Particle;
IMP_END_NAMESPACE

IMP_BEGIN_INTERNAL_NAMESPACE



template <class T>
struct ListContains{
  const T& t_;
  ListContains(const T &t): t_(t){}
  bool operator()(typename T::value_type t) const {
    return std::binary_search(t_.begin(), t_.end(), t);
  }
};

template <class T>
ListContains<T> list_contains(const T&t) {
  return ListContains<T>(t);
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_VECTOR_H */
