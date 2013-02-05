/**
 *  \file IndexingIterator.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INDEXING_ITERATOR_H
#define IMPKERNEL_INDEXING_ITERATOR_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/check_macros.h>
#include <iterator>
#include <limits>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Accessor>
class IndexingIterator {
  Accessor a_;
  unsigned int i_;
  mutable typename Accessor::result_type vt_;
  typedef IndexingIterator<Accessor> This;
  bool is_default() const {
    return i_!= std::numeric_limits<unsigned int>::max();
  }
public:
  typedef const typename Accessor::result_type  value_type;
  typedef int difference_type;
  typedef const typename Accessor::result_type reference;
  typedef const typename Accessor::result_type* pointer;
  typedef std::random_access_iterator_tag iterator_category;

  IndexingIterator(): i_(std::numeric_limits<unsigned int>::max()){}
  IndexingIterator(Accessor a): a_(a), i_(0){}
  IndexingIterator(Accessor a,
                   unsigned int i): a_(a), i_(i){}

  const This& operator++() {
    ++i_;
    return *this;
  }

  This operator++(int) {
    This o= *this;
   ++i_;
    return o;
  }

  const This& operator--() {
    --i_;
    return *this;
  }

  This operator--(int) {
    This o= *this;
    --i_;
    return o;
  }

  This operator+( int i) const {
    return This(a_, i_+i);
  }

  This operator-( int i) const {
    return This(a_, i_-i);
  }

  const This& operator+=( int i) {
    i_+= i;
    return *this;
  }

  const This& operator-=( int i) {
    i_-= i;
    return *this;
  }

  unsigned int operator-(const This &o) const {
    IMP_INTERNAL_CHECK(a_== o.a_,
               "Don't subtract iterators from different containers");
    return i_- o.i_;
  }

  bool operator==(const This &o) const {
    IMP_INTERNAL_CHECK(a_ == o.a_,
               "Can only compare iterators from the same container");
    return (i_== o.i_);
  }

  bool operator!=(const This &o) const {
    return (i_!= o.i_);
  }

  bool operator<(const This &o) const {
    IMP_INTERNAL_CHECK(a_ == o.a_,
               "Can only compare iterators from the same container");
    IMP_INTERNAL_CHECK(!is_default() && !o.is_default(),
               "Ordering with uninitialized index is undefined");
    return (i_< o.i_);
  }

  bool operator>(const This &o) const {
    IMP_INTERNAL_CHECK(a_ == o.a_,
               "Can only compare iterators from the same container");
    IMP_INTERNAL_CHECK(!is_default() && !o.is_default(),
               "Ordering with uninitialized index is undefined");
    return (i_> o.i_);
  }

  bool operator>=(const This &o) const {
    IMP_INTERNAL_CHECK(a_ == o.a_,
               "Can only compare iterators from the same container");
    IMP_INTERNAL_CHECK(!is_default() && !o.is_default(),
               "Ordering with uninitialized index is undefined");
    return (i_>= o.i_);
  }

  bool operator<=(const This &o) const {
    IMP_INTERNAL_CHECK(a_ == o.a_,
               "Can only compare iterators from the same container");
    IMP_INTERNAL_CHECK(!is_default() && !o.is_default(),
               "Ordering with uninitialized index is undefined");
    return (i_<= o.i_);
  }

  reference operator[](unsigned int i) {
    This t=*this+i;
    return *t;
  }

  reference operator*() const {
    return a_(i_);
  }

  pointer operator->() const {
    vt_= a_(i_);
    return &vt_;
  }
};


/*template <class A>
IndexingIterator<A> operator-(int i,
                              IndexingIterator<A> in) {
  return in+ (-i);
  }*/

template <class A>
inline IndexingIterator<A> operator+(int i,
                                     IndexingIterator<A> in) {
  return in+ (i);
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INDEXING_ITERATOR_H */
