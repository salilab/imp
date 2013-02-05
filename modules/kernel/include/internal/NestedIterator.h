/**
 *  \file NestedIterator.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_NESTED_ITERATOR_H
#define IMPKERNEL_INTERNAL_NESTED_ITERATOR_H
#include <IMP/kernel/kernel_config.h>
#include "../exception.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class T>
class NestedIterator {
  typedef NestedIterator<T> This;
  typedef typename T::Make_value Make_value;
  typedef typename T::Get_inner Get_inner;
  typedef typename T::Inner Inner;
  typedef typename T::Outer Outer;
public:
  typedef typename T::value_type value_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef int difference_type;
  typedef value_type reference;
  typedef value_type* pointer;

  reference operator*() {
    return ret_;
  }
  const reference operator*() const {
    return ret_;
  }
  pointer operator->() {
    return &ret_;
  }
  const This& operator++() {
    IMP_INTERNAL_CHECK(ait_ != aend_, "Empty range");
    ++ait_;
    while (ait_== aend_) {
      ++rit_;
      if (rit_== rend_) {
        break;
      }
      ait_= Get_inner()(rit_).first;
      ait_= Get_inner()(rit_).second;
    }
    if (rit_ != rend_) {
      ret_=Make_value()(rit_, ait_);
    }
    return *this;
  }

  This operator++(int) {
    This r= *this;
    operator++();
    return r;
  }

  bool operator==(const This& o) const {
    if (rit_ == rend_) return rit_==o.rit_;
    else return rit_== o.rit_ && ait_ == o.ait_;
  }
  bool operator!=(const This& o) const {
    if (rit_== rend_) return rit_!= o.rit_;
    else return rit_!= o.rit_ || ait_ != o.ait_;
  }
  NestedIterator(){}
  template <class OT>
  NestedIterator(const NestedIterator<OT> &o) {
    copy_from(o);
  }
  template <class OT>
  This& operator=(const NestedIterator<OT> &o) {
    copy_from(o);
    return *this;
  }
  NestedIterator(const This &o) {
    copy_from(o);
  }
  This& operator=(const This &o) {
    copy_from(o);
    return *this;
  }

  template <class It>
  NestedIterator(It b, It e): rit_(b), rend_(e){
    if (b != e) {
      ait_=Get_inner()(rit_).first;
      aend_= Get_inner()(rit_).second;
      //typename boost::tuple_element<1, value_type>::type st=ait_->second;
      ret_= Make_value()(rit_, ait_);
    }
  }

protected:
  template <class OT>
  void copy_from(const NestedIterator<OT> &o) {
    rit_= o.rit_;
    rend_= o.rend_;
    if (rit_!= rend_) {
      ait_=o.ait_;
      aend_=o.aend_;
      ret_= o.ret_;
    }
  }



  typename T::Outer rit_, rend_;
  typename T::Inner ait_, aend_;
  value_type ret_;
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_NESTED_ITERATOR_H */
