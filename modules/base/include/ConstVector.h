/**
 *  \file IMP/base/ConstVector.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CONST_VECTOR_H
#define IMPBASE_CONST_VECTOR_H

#include <IMP/base/base_config.h>
#include "base_macros.h"
#include "exception.h"
#include "Value.h"
#include <IMP/base/nullptr.h>
#include <boost/scoped_array.hpp>
#include <IMP/base/hash.h>
#include <iterator>

IMPBASE_BEGIN_NAMESPACE

//! Store an array of values of the same type.
/** Items must be comparable and hashable and the arrays
    cannote be changed after creation.*/
template <class Data, class SwigData=Data>
class ConstVector: public Value {
  boost::scoped_array<Data> v_;
  unsigned int sz_;
  int compare(const ConstVector &o) const {
    if (size() < o.size()) return -1;
    else if (size() > o.size()) return 1;
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o.v_[i]) return -1;
      else if (v_[i] > o.v_[i]) return 1;
    }
    return 0;
  }
  void create(unsigned int sz) {
    if (sz==0) {
      v_.reset(nullptr);
    } else {
      v_.reset(new Data[sz]);
    }
    sz_=sz;
  }
  template <class It>
  void copy_from(It b, It e) {
    create(std::distance(b,e));
    std::copy(b,e, v_.get());
  }
public:
  ~ConstVector() {
  }
  ConstVector(unsigned int sz, Data fill) {
    create(sz);
    std::fill(v_.get(), v_.get()+sz, fill);
  }
  ConstVector(): v_(0), sz_(0){}
  template <class It>
  ConstVector(It b, It e) {
    copy_from(b,e);
  }
  template <class Vector>
  explicit ConstVector(const Vector &i) {
    copy_from(i.begin(), i.end());
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  /* Add the arguments to attempt to make VC happy as it tries to
     use the templated version instead.
   */
  ConstVector(const ConstVector<Data, SwigData> &o): Value(), sz_(0) {
    copy_from(o.begin(), o.end());
  }
  ConstVector<Data, SwigData>& operator=(const ConstVector<Data, SwigData> &o) {
    copy_from(o.begin(), o.end());
    return *this;
  }
  ConstVector(int sz) {
    create(sz);
  }
#endif
  IMP_COMPARISONS(ConstVector);
#ifndef SWIG
  Data operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < sz_, "Out of range");
    return v_[i];
  }
#ifndef IMP_DOXYGEN
  void set_item(unsigned int i, SwigData v) const {
    IMP_USAGE_CHECK(i < sz_, "Out of range");
    v_[i]=v;;
  }
#endif
#endif
#ifndef IMP_DOXYGEN
  SwigData __getitem__(unsigned int i) const {
    if (i >= sz_) IMP_THROW("Out of bound " << i << " vs " << sz_,
                            IndexException);
    return operator[](i);
  }
  unsigned int __len__() const {return sz_;}
#endif
#ifndef SWIG
  unsigned int size() const {
    return sz_;
  }
#endif
  IMP_SHOWABLE_INLINE(ConstVector, {
      out << "(";
      for (unsigned int i=0; i< size(); ++i) {
        out << Showable(v_[i]);
        if (i != size()-1) out << " ";
      }
      out << ")";
    });
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  typedef const Data * const_iterator;
  const_iterator begin() const {
    return v_.get();
  }
  const_iterator end() const {
    return v_.get()+size();
  }
  void swap_with(ConstVector &o) {
    std::swap(sz_, o.sz_);
    v_.swap(o.v_);
  }
#endif
  IMP_HASHABLE_INLINE(ConstVector, return boost::hash_range(begin(),
                                                           end()););
};

IMP_SWAP_1(ConstVector);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class D>
inline std::size_t hash_value(const ConstVector<D> &t) {
  return t.__hash__();
}
#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONST_VECTOR_H */
