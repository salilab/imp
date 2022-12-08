/**
 *  \file IMP/Vector.h
 *  \brief A class for storing lists of IMP items.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONVERTIBLE_VECTOR_H
#define IMPKERNEL_CONVERTIBLE_VECTOR_H
#include <IMP/kernel_config.h>
// do not include anything more from base
#include "Showable.h"
#include "Value.h"
#include <sstream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include "hash.h"

#if defined(_MSC_VER) && _MSC_VER == 1500
# include <boost/type_traits.hpp>
# include <boost/utility.hpp>
#endif

#if IMP_COMPILER_HAS_DEBUG_VECTOR &&IMP_HAS_CHECKS >= IMP_INTERNAL
#include <debug/vector>
#else
#include <vector>
#endif

IMPKERNEL_BEGIN_NAMESPACE

//! A more \imp-like version of the \c std::vector.
/** Specifically this class adds functionality from \c Python arrays such as
    - hashing
    - output to streams
    - use of \c +=es
    - implicit conversion when the contents are implicitly convertible
    - bounds checking in debug mode
 */
template <class T>
class Vector : public Value
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#if IMP_COMPILER_HAS_DEBUG_VECTOR &&IMP_HAS_CHECKS >= IMP_INTERNAL
               ,
               public __gnu_debug::vector<T>
#else
               ,
               public std::vector<T>
#endif
#endif
               {
#if IMP_COMPILER_HAS_DEBUG_VECTOR &&IMP_HAS_CHECKS >= IMP_INTERNAL
  typedef __gnu_debug::vector<T> V;
#else
  typedef std::vector<T> V;
#endif

#ifndef SWIG
  friend class boost::serialization::access;

  template<class Archive> void save(Archive &ar, const unsigned int) const {
    size_t sz = V::size();
    ar << sz;
    auto it = V::begin();
    while(sz-- > 0) {
      ar << *it++;
    }
  }

  template<class Archive> void load(Archive &ar, const unsigned int) {
    size_t sz;
    ar >> sz;
    V::resize(sz);
    auto it = V::begin();
    while(sz-- > 0) {
      ar >> *it++;
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

 public:
  Vector() {}
  explicit Vector(unsigned int sz, const T &t = T()) : V(sz, t) {}
#if defined(_MSC_VER) && _MSC_VER == 1500
  template <class It>
  Vector(It b, It e,
         typename boost::disable_if<boost::is_integral<It> >::type *t=0) {
    for (It it = b; it != e; ++it) {
      push_back(T(*it));
    }
  }
  template <class VO>
  explicit Vector(const std::vector<VO> &o) {
    reserve(o.size());
    for (std::vector<VO>::const_iterator it = o.begin();
         it != o.end(); ++it) {
      push_back(T(*it));
    }
  }
#else
  template <class It>
  Vector(It b, It e)
      : V(b, e) {}
  template <class VO>
  explicit Vector(const std::vector<VO> &o)
      : V(o.begin(), o.end()) {}
#endif
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
  void show(std::ostream &out = std::cout) const {
    out << "[";
    for (unsigned int i = 0; i < V::size(); ++i) {
      if (i > 0) out << ", ";
      if (i > 10) {
        out << ",...";
        break;
      }
      out << Showable(V::operator[](i));
    }
    out << "]";
  }
  operator Showable() const {
    std::ostringstream oss;
    show(oss);
    return Showable(oss.str());
  }
  std::size_t __hash__() const {
    return boost::hash_range(V::begin(), V::end());
  }
#endif
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class T>
void swap(Vector<T> &a, Vector<T> &b) {
  a.swap(b);
}

template <class T>
inline Vector<T> operator+(Vector<T> ret, const Vector<T> &o) {
  ret.insert(ret.end(), o.begin(), o.end());
  return ret;
}

#endif

#if IMP_COMPILER_HAS_DEBUG_VECTOR &&IMP_HAS_CHECKS >= IMP_INTERNAL
template <class T>
inline std::size_t hash_value(const __gnu_debug::vector<T> &t) {
  return boost::hash_range(t.begin(), t.end());
}
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONVERTIBLE_VECTOR_H */
