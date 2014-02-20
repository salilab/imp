/**
 *  \file IMP/algebra/algebra_macros.h    \brief Various important macros
 *                           for implementing geometry.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_MACROS_H
#define IMPALGEBRA_MACROS_H
#include "geometric_primitive_macros.h"
#include <boost/range/iterator_range.hpp>

#ifdef SWIG
#define IMP_ALGEBRA_VECTOR_SWIG_METHODS(D) \
  VectorD(const Floats &f);
#else
#define IMP_ALGEBRA_VECTOR_SWIG_METHODS(D) \
  template <class R>                                                          \
  VectorD &operator=(const R &o) {                                            \
    P::operator=(o);                                                          \
    return *this;                                                             \
  }

#endif

#define IMP_ALGEBRA_VECTOR_METHODS(D)                                         \
  typedef VectorBaseD<D> P;                                                   \
                                                                              \
 public:                                                                      \
  /** Will accept a list of floats from python. */                            \
  template <class Range>                                                      \
  explicit VectorD(Range r)                                                   \
      : P(r) {}                                                               \
  template <int OD>                                                           \
  VectorD(const VectorD<OD> &o)                                               \
      : P(o) {                                                                \
    BOOST_STATIC_ASSERT(OD == D || OD == -1 || D == -1);                      \
  }                                                                           \
  IMP_ALGEBRA_VECTOR_SWIG_METHODS(D);                                         \
  /** The distance between b and e must be equal to D.                        \
   */                                                                         \
  template <class It>                                                         \
  VectorD(It b, It e)                                                         \
      : P(boost::make_iterator_range(b, e)) {}                                \
  VectorD() {}                                                                \
  VectorD get_unit_vector() const { return algebra::get_unit_vector(*this); } \
  VectorD operator*(double s) const {                                         \
    VectorD ret(*this);                                                       \
    ret *= s; return ret;						\
  }                                                                           \
  VectorD operator/(double s) const {                                         \
    VectorD ret(*this);                                                       \
    ret /= s; return ret;					\
  }                                                                           \
  VectorD operator-() const {                                                 \
    VectorD ret(*this);                                                       \
    ret *= -1; return ret;						\
  }                                                                           \
  VectorD operator-(const VectorD &o) const {                                 \
    VectorD ret(*this); ret -= o; return ret;			\
  }                                                                           \
  VectorD operator+(VectorD ret) const {				\
    ret += *this; return ret;						\
  }                                                                           \
  VectorD &operator+=(const VectorD& o) {                                     \
    P::operator+=(o);                                                         \
    return *this;                                                             \
  }                                                                           \
  VectorD &operator-=(const VectorD &o) {                                     \
    P::operator-=(o);                                                         \
    return *this;                                                             \
  }                                                                           \
  VectorD &operator/=(double f) {                                             \
    P::operator/=(f);                                                         \
    return *this;                                                             \
  }                                                                           \
  VectorD &operator*=(double f) {                                             \
    P::operator*=(f);                                                         \
    return *this;                                                             \
  }                                                                           \
  /* for swig as it doesn't like using. */                                    \
  double operator*(const VectorD &o) const { return P::operator*(o); }

#endif /* IMPALGEBRA_MACROS_H */
