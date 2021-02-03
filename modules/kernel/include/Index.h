/**
 *  \file IMP/Index.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INDEX_H
#define IMPKERNEL_INDEX_H

#include <IMP/kernel_config.h>
#include "bracket_macros.h"
#include "showable_macros.h"
#include "Value.h"
#include <IMP/Vector.h>

IMPKERNEL_BEGIN_NAMESPACE

//! A typed index.
/** This can help disambiguate different integer based indexes floating
    around to help avoid bugs caused by mixing them up. Care has been taken
    so that it can be replaced by an integer everywhere, if needed. */
template <class Tag>
class Index : public Value {
  int i_;

 public:
  explicit Index(int i) : i_(i) {}
  Index() : i_(-2) {}
  int get_index() const {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    IMP_INTERNAL_CHECK(i_ >= 0, "Invalid index");
    return i_;
  }
  IMP_COMPARISONS_1(Index, i_);
  IMP_SHOWABLE_INLINE(Index, {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    out << i_;
  });
  IMP_HASHABLE_INLINE(Index, {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    return i_;
  });
};
template <class Tag>
inline unsigned int get_as_unsigned_int(Index<Tag> i) {
  return i.get_index();
}
template <class Tag>
inline Index<Tag> get_invalid_index() {
  return Index<Tag>(-1);
}

/** This class implements a vector tied to a particular index
    of type Index<Tag>.
 */
template <class Tag, class T>
class IndexVector : public Vector<T> {
  typedef Vector<T> P;

 public:
  IndexVector(unsigned int sz, const T &t = T()) : P(sz, t) {}
  IndexVector() {}
  IMP_BRACKET(T, Index<Tag>, get_as_unsigned_int(i) < P::size(),
              return P::operator[](get_as_unsigned_int(i)));
};

template <class Tag, class Container, class T>
void resize_to_fit(Container &v, Index<Tag> i, const T &default_value = T()) {
  if (v.size() <= get_as_unsigned_int(i)) {
    v.resize(get_as_unsigned_int(i) + 1, default_value);
  }
}

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_INDEX_H */
