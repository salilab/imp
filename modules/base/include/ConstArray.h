/**
 *  \file base/ConstArray.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CONST_ARRAY_H
#define IMPBASE_CONST_ARRAY_H

#include "base_config.h"
#include "base_macros.h"
#include "exception.h"
#include <boost/scoped_array.hpp>
#include <IMP/compatibility/hash.h>

IMPBASE_BEGIN_NAMESPACE

//! Store an array of values of the same type.
/** Items must be comparable and hashable and the arrays
    cannote be changed after creation.*/
template <class Data, class SwigData=Data>
class ConstArray {
  boost::scoped_array<Data> v_;
  unsigned int sz_;
  int compare(const ConstArray &o) const {
    if (size() < o.size()) return -1;
    else if (size() > o.size()) return 1;
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o.v_[i]) return -1;
      else if (v_[i] > o.v_[i]) return 1;
    }
    return 0;
  }
  void create(unsigned int sz) {
    IMP_USAGE_CHECK(sz>0, "can't create 0 size subset");
    v_.reset(new Data[sz]);
    sz_=sz;
  }
  void copy_from(const ConstArray &o) {
    sz_=o.sz_;
    v_.reset(new Data[sz_]);
    std::copy(o.v_.get(), o.v_.get()+sz_, v_.get());
  }
public:
  ~ConstArray() {
  }
  ConstArray(unsigned int sz, Data fill) {
    create(sz);
    std::fill(v_.get(), v_.get()+sz, fill);
  }
  ConstArray(): v_(0), sz_(0){}
  template <class It>
  ConstArray(It b, It e) {
    IMP_USAGE_CHECK(std::distance(b,e) > 0,
                    "Can't create ConstArray from empty list");
    create(std::distance(b,e));
    std::copy(b,e, v_.get());
  }
  template <class Vector>
  explicit ConstArray(const Vector &i) {
    IMP_USAGE_CHECK(!i.empty(),
                    "Can't create ConstArray from empty list");
    create(i.size());
    std::copy(i.begin(), i.end(), v_.get());
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  ConstArray(const ConstArray &o): sz_(0) {
    copy_from(o);
  }
  ConstArray& operator=(const ConstArray &o) {
    copy_from(o);
    return *this;
  }
  ConstArray(int sz) {
    create(sz);
  }
#endif
  IMP_COMPARISONS(ConstArray);
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
  IMP_SHOWABLE_INLINE(ConstArray, {
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
  void swap_with(ConstArray &o) {
    std::swap(sz_, o.sz_);
    v_.swap(o.v_);
  }
#endif
  IMP_HASHABLE_INLINE(ConstArray, return boost::hash_range(begin(),
                                                       end()););
};

IMP_SWAP_1(ConstArray);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class D>
inline std::size_t hash_value(const ConstArray<D> &t) {
  return t.__hash__();
}
#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CONST_ARRAY_H */
