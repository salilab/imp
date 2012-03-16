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
#include "Showable.h"
#include <sstream>
#include <IMP/compatibility/hash.h>

IMPBASE_BEGIN_NAMESPACE

/** This class provides a more \imp-like version of the \c std::vector.
    Specifically it adds functionality from \c Python arrays such as
    - hashing
    - output to streams
    - use of \c +=es
    - implicit conversion when the contents are implicitly convertible
    - bounds checking in debug mode
*/
template <class T>
class Vector: public compatibility::vector<T> {
  typedef compatibility::vector<T> V;
 public:
  Vector(){}
  Vector(unsigned int sz, const T&t=T()): V(sz, t){}
  template <class It>
  Vector(It b, It e): V(b,e){}
  template <class O>
  Vector(const compatibility::vector<O> &o): V(o.begin(),
                                                      o.end()){}
   template <class O>
  operator Vector<O>() const {
    return Vector<O>(V::begin(), V::end());
  }
  template <class OV>
  base::Vector<T> operator+=(const OV &o) {
    V::insert(V::end(), o.begin(), o.end());
    return *this;
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void show(std::ostream &out=std::cout) const{
    out << "[";
    for (unsigned int i=0; i< V::size(); ++i) {
      if (i >0) out << ", ";
      out << Showable(V::operator[](i));
    }
    out<< "]";
  }
  operator Showable() const {
    std::ostringstream oss;
    show(oss);
    return Showable(oss.str());
  }
  std::size_t __hash__() const {
    return boost::hash_range(V::begin(),
                             V::end());
  }
#endif
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class T>
void swap(base::Vector<T> &a,
          base::Vector<T> &b) {
  std::swap(static_cast<IMP::base::Vector<T> &>(a),
            static_cast<IMP::base::Vector<T> &>(b));
}
template <class T>
inline base::Vector<T> operator+( base::Vector<T> ret,
                                  const base::Vector<T> &o) {
  ret.insert(ret.end(), o.begin(), o.end());
  return ret;
}

#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONVERTIBLE_VECTOR_H */
