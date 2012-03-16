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
#include <IMP/compatibility/hash.h>
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

/** This class provides a more \imp-like version of the \c std::vector.
    Specifically it adds functionality from \c Python arrays such as
    - hashing
    - output to streams
    - use of \c +=es
    - implicit conversion when the contents are implicitly convertible
*/
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
  template <class OV>
  ConvertibleVector<T> operator+=(const OV &o) {
    V::insert(V::end(), o.begin(), o.end());
    return *this;
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void show(std::ostream &out=std::cout) const{
    out << Showable(*this);
  }
  operator Showable() const {
    return Showable(static_cast<V>(*this));
  }
  std::size_t __hash__() const {
    return boost::hash_range(V::begin(),
                             V::end());
  }
#endif
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class T>
void swap(vector<T> &a,
          vector<T> &b) {
  std::swap(static_cast<IMP::vector<T> &>(a),
            static_cast<IMP::vector<T> &>(b));
}
template <class T>
inline ConvertibleVector<T> operator+( ConvertibleVector<T> ret,
                                      const ConvertibleVector<T> &o) {
  ret.insert(ret.end(), o.begin(), o.end());
  return ret;
}

#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONVERTIBLE_VECTOR_H */
