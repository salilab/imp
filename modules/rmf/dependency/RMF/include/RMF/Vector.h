/**
 *  \file RMF/Vector.h
 *  \brief Represent coordinates.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_VECTOR_H
#define RMF_VECTOR_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <type_traits>
#include <array>
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {

#ifndef SWIG
/** \brief Represent a point in some dimension.

    std::array provides `operator[]()` and `begin()`/`end()` in C++.
 */
template <unsigned int D>
class Vector
    : public std::array<float, D>
      {
  typedef std::array<float, D> P;
  // work around swig
  template <class R, class Enabled = void>
  struct Convert {};

  template <class R>
  struct Convert<R, typename std::enable_if<std::is_convertible<
                        R, std::array<float, D> >::value>::type> {
    static void convert(const R& r, std::array<float, D>& d) { d = r; }
  };

  template <class R>
  struct Convert<R, typename std::enable_if<!std::is_convertible<
                        R, std::array<float, D> >::value>::type> {
    static void convert(const R& r, std::array<float, D>& d) {
      std::copy(boost::begin(r), boost::end(r), d.begin());
    }
  };
 public:
  Vector() {}
  //#ifndef RMF_SWIG_WRAPPER
  template <class Range>
  explicit Vector(Range r) {
    Convert<Range>::convert(r, *this);
  }

  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(Vector);

  Vector(float x, float y, float z) {
    static_assert(D == 3);
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
  }
  Vector(float x, float y, float z, float q) {
    static_assert(D == 4);
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
    P::operator[](3) = q;
  }
  RMF_SHOWABLE(Vector, std::vector<float>(P::begin(), P::end()));
#if !defined(RMF_DOXYGEN)
  float __getitem__(unsigned int i) const {
    if (i >= D) throw IndexException();
    return P::operator[](i);
  }
  unsigned int __len__() const { return D; }
#endif
  static unsigned int get_dimension() { return D; }
};

#else
template <unsigned int D>
class Vector {};
template <>
struct Vector<3U> {
  Vector() {}
  Vector(const Vector<3U> &o);
  Vector(float x, float y, float z);
  float __getitem__(unsigned int i) const;
  unsigned int __len__() const;
  static unsigned int get_dimension();
};
template <>
struct Vector<4U> {
  Vector() {}
  Vector(const Vector<4U> &o);
  Vector(float w, float x, float y, float z);
  float __getitem__(unsigned int i) const;
  unsigned int __len__() const;
  static unsigned int get_dimension();
};
#endif

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_VECTOR_H */
