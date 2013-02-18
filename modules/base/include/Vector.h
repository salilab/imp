/**
 *  \file IMP/base/Vector.h
 *  \brief A class for storing lists of IMP items.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CONVERTIBLE_VECTOR_H
#define IMPBASE_CONVERTIBLE_VECTOR_H
#include <IMP/base/base_config.h>
// do not include anything more from base
#include "Showable.h"
#include "Value.h"
#include <sstream>
#include "hash.h"

#if IMP_COMPILER_HAS_DEBUG_VECTOR && IMP_HAS_CHECKS >= IMP_INTERNAL
#include <debug/vector>
#else
#include <vector>
#endif


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
class Vector: public Value
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#if IMP_COMPILER_HAS_DEBUG_VECTOR && IMP_HAS_CHECKS >= IMP_INTERNAL
  , public __gnu_debug::vector<T>
#else
  , public  std::vector<T>
#endif
#endif
 {
#if IMP_COMPILER_HAS_DEBUG_VECTOR && IMP_HAS_CHECKS >= IMP_INTERNAL
   typedef __gnu_debug::vector<T> V;
#else
   typedef std::vector<T> V;
#endif
 public:
  Vector(){}
  explicit Vector(unsigned int sz, const T&t=T()): V(sz, t){}
  template <class It>
  Vector(It b, It e): V(b,e){}
  template <class VO>
  explicit Vector(const std::vector<VO> &o): V(o.begin(), o.end()){}
   template <class O>
  operator Vector<O>() const {
    return Vector<O>(V::begin(), V::end());
  }
  template <class OV>
  Vector<T> operator+=(const OV &o) {
    V::insert(V::end(), o.begin(), o.end());
    return *this;
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void show(std::ostream &out=std::cout) const{
    out << "[";
    for (unsigned int i=0; i< V::size(); ++i) {
      if (i >0) out << ", ";
      if (i > 10) {
        out << ",...";
        break;
      }
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
void swap(Vector<T> &a,
          Vector<T> &b) {
  a.swap(b);
}

template <class T>
inline Vector<T> operator+( Vector<T> ret,
                                  const Vector<T> &o) {
  ret.insert(ret.end(), o.begin(), o.end());
  return ret;
}

#endif

#if IMP_COMPILER_HAS_DEBUG_VECTOR && IMP_HAS_CHECKS >= IMP_INTERNAL
template <class T>
inline std::size_t hash_value(const __gnu_debug::vector<T> &t) {
  return boost::hash_range(t.begin(), t.end());
}
#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONVERTIBLE_VECTOR_H */
