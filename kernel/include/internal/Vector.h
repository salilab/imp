/**
 *  \file Vector.h    \brief A bounds checked vector.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VECTOR_H
#define __IMP_VECTOR_H

#include "../exception.h"
#include "../Object.h"
#include "../RefCountedObject.h"
#include "ref_counting.h"

#include <vector>

namespace IMP
{

class Particle;

namespace internal
{

//! A vector with bounds checking
/** This class is designed to be used from the IMP_LIST macro and
    is not really designed to be used independently.
    \internal
 */
template <class D>
class Vector: public std::vector<D>
{
public:
  typedef std::vector<D> P;
  Vector(){}

  template <class It>
  Vector(It b, It e) {
    insert(P::end(), b, e);
  }

  Vector(const P &p) {
    insert(P::end(), p.begin(), p.end());
  }

  ~Vector(){clear();}

  const D& operator[](unsigned int i) const {
    IMP_check(i < P::size(),
              "Index " << i << " out of range",
              IndexException);
    return P::operator[](i);
  }

  D& operator[](unsigned int i) {
    IMP_check(i < P::size(),
              "Index " << i << " out of range",
              IndexException);
    return P::operator[](i);
  }

  void erase(typename P::iterator it) {
    unref(*it);
    P::erase(it);
  }

  void erase(typename P::iterator b,
             typename P::iterator e) {
    for (typename P::iterator c= b; c != e; ++c) {
      unref(*c);
    }
    P::erase(b,e);
  }

  unsigned int push_back(D d) {
    ref(d);
    P::push_back(d);
    return P::size()-1;
  }

  void pop_back() {
    unref(P::back());
    P::pop_back();
  }

  void clear() {
    for (typename P::iterator it= P::begin(); it != P::end(); ++it) {
      unref(*it);
    }
    P::clear();
  }

  template <class It>
  void insert(typename P::iterator it, It b, It e) {
    for (It c= b; c != e; ++c) {
      D d=*c;
      ref(d);
    }
    P::insert(it, b, e);
  }
};




} // namespace internal

} // namespace IMP

#endif  /* __IMP_VECTOR_H */
