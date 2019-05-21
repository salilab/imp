/**
 *  \file RMF/Vector.h
 *  \brief Represent coordinates.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_VECTOR_H
#define RMF_VECTOR_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/mpl/not.hpp>
#include <boost/array.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {

#ifndef SWIG
/** \brief Represent a point in some dimension.

    [boost::array](http://www.boost.org/doc/libs/1_55_0/doc/html/array.html)
    provides `operator[]()` and `begin()`/`end()` in C++.
 */
template <unsigned int D>
class Vector
    : public boost::array<float, D>
      {
  typedef boost::array<float, D> P;
  // work around swig
  template <class R, class Enabled = void>
  struct Convert {};

  template <class R>
  struct Convert<R, typename boost::enable_if<boost::is_convertible<
                        R, boost::array<float, D> > >::type> {
    static void convert(const R& r, boost::array<float, D>& d) { d = r; }
  };

  template <class R>
  struct Convert<
      R, typename boost::enable_if<boost::mpl::not_<
             boost::is_convertible<R, boost::array<float, D> > > >::type> {
    static void convert(const R& r, boost::array<float, D>& d) {
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
    BOOST_STATIC_ASSERT(D == 3);
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
  }
  Vector(float x, float y, float z, float q) {
    BOOST_STATIC_ASSERT(D == 4);
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
