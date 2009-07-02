/**
 *  \file RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTED_H
#define IMP_REF_COUNTED_H

#include "config.h"
#include "exception.h"
#include "log.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <vector>

#ifndef IMP_DOXYGEN
#ifndef SWIG

namespace IMP {
  class RefCounted;
  namespace internal {
    template <class R>
    void do_unref(R*);
    void do_ref(IMP::RefCounted*);
  }
}
//IMP_END_INTERNAL_NAMESPACE
#endif
#endif

IMP_BEGIN_NAMESPACE

//! Common base class for ref counted objects.
/** This base class implements reference counting when used in
    conjunction with IMP::Pointer or IMP::WeakPointer objects.
    Objects which inherit from IMP::RefCounted should be passed
    using pointers and stored using IMP::Pointer and
    IMP::WeakPointer objects. Users must be careful to avoid
    cycles of reference counted pointers, otherwise memory will
    never be reclaimed.

    \par Introduction to reference counting:
    Reference counting is a technique for managing memory and
    automatically freeing memory (destroying objects) when it
    is no longer needed. It is used by Python to do its memory
    management. In reference counting, each object has a reference
    count, which tracks how many different places are using the
    object. When this count goes to 0, the object is freed. The
    IMP::Pointer class adds one to the reference count of the
    IMP::RefCounted object it points to (and subtracts one when
    it no longer points to the object.\n\n Be aware of cycles,
    since if, for example, object A contains an IMP::Pointer to
    object B and object B contains an IMP::Pointer to object A,
    their reference counts will never go to 0 even if both A
    and B are no longer used. To avoid this, use an
    IMP::WeakPointer in one of A or B.

    IMP::RefCounted provides no public methods or constructors.
    It makes objects that inherit from it non-copyable.

    \see IMP_REF_COUNTED_DESTRUCTOR()
 */
class IMPEXPORT RefCounted
{
  typedef RefCounted This;
  static unsigned int live_objects_;
  mutable int count_;
  RefCounted(const RefCounted &){}
  RefCounted& operator=(const RefCounted &){return *this;}

#ifndef IMP_DOXYGEN
  template <class R>
    friend void internal::do_unref(R*);
  friend void internal::do_ref(RefCounted*);
protected:
  RefCounted() {
     ++live_objects_;
     count_=0;
  }
  ~RefCounted();

 public:
  unsigned int get_ref_count() const {
    return count_;
  }

  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }
#endif

};

IMP_END_NAMESPACE

#include "internal/ref_counting.h"



IMP_BEGIN_NAMESPACE

//! A vector of ref counted pointers
/** Documentation for std::vector can be found at as part of the SGI
    stl documentation, among other places
    (http://www.sgi.com/tech/stl/Vector.html).

    When used within Python, IMP::VectorOfRefCounted acts like a Python list.
 */
template <class RC>
class VectorOfRefCounted {
  typedef std::vector<RC> Data;
  Data data_;
  // make sure they are converted to RC
  void ref(RC v) {
    internal::ref(v);
  }
  void unref(RC v) {
    internal::unref(v);
  }
  template <class It>
    void ref(It b, It e) {
    for (It c= b; c != e; ++c) {
      ref(*c);
    }
  }
  template <class It>
    void unref(It b, It e) {
    for (It c= b; c != e; ++c) {
      unref(*c);
    }
  }
 public:
  typedef RC const_reference;
  typedef RC value_type;
  template <class It>
  VectorOfRefCounted(It b, It e): data_(b,e){
    ref(b,e);
  }
  VectorOfRefCounted(const VectorOfRefCounted<RC> &o):
    data_(o.begin(), o.end()){
    ref(o.begin(), o.end());
  }
  VectorOfRefCounted(RC rc): data_(1, rc) {
    ref(rc);
  }
  VectorOfRefCounted(unsigned int i): data_(i, NULL){}
  VectorOfRefCounted(){}
  ~VectorOfRefCounted() {
    clear();
  }
#ifndef SWIG
  const VectorOfRefCounted<RC> operator=(const VectorOfRefCounted<RC> &o) {
    unref(data_.begin(), data_.end());
    data_= o.data_;
    ref(data_.begin(), data_.end());
    return *this;
  }
#endif
  RC operator[](unsigned int i) const {
    IMP_check(i < size(), "Index out of range", IndexException);
    return data_[i];
  }
  void set(unsigned int i, RC p) {
    IMP_check(i < size(), "Index out of range", IndexException);
    if (data_[i]) unref(data_[i]);
    data_[i]=p;
    ref(p);
  }
  RC back() const {
    IMP_check(!empty(), "Can't call back on empty container",
              InvalidStateException);
    return data_.back();
  }
  RC front() const {
    IMP_check(!empty(), "Can't call front on empty container",
              InvalidStateException);
    return data_.front();
  }
  void reserve(unsigned int i){ data_.reserve(i);}
  unsigned int size() const {return data_.size();}
  void resize(unsigned int i) {data_.resize(i, NULL);}
  // god swig is dumb
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  typedef typename Data::iterator iterator;
  typedef typename Data::const_iterator const_iterator;
#else
  class iterator;
  class const_iterator;
#endif
  const_iterator begin() const {return data_.begin();}
  const_iterator end() const {return data_.end();}
  iterator begin() {return data_.begin();}
  iterator end() {return data_.end();}
  template <class It>
  void insert(iterator loc, It b, It e) {
    data_.insert(data_.begin()+(loc-data_.begin()), b, e);
    ref(b,e);
  }
  void push_back(RC p) {
    data_.push_back(p);
    ref(p);
  }
  bool empty() const {return data_.empty();}
#ifndef IMP_DOXYGEN
  void swap_with(VectorOfRefCounted<RC> &a) {
    std::swap(a.data_, data_);
  }
#endif
  void clear(){
    unref(data_.begin(), data_.end());
    data_.clear();
  }
  void show(std::ostream &out) const {
    for (unsigned int i=0; i< size(); ++i) {
      if (operator[](i)) {
        out << operator[](i)->get_name() << " ";
      } else {
        out << "NULL ";
      }
    }
  }
  void erase(iterator it) {
    unref(*it);
    data_.erase(it);
  }
  void erase(iterator b, iterator e) {
    unref(b, e);
    data_.erase(b, e);
  }
  template <class F>
  void remove_if(const F &f) {
    for (iterator it= begin(); it != end(); ++it) {
      if (f(*it)) unref(*it);
    }
    data_.erase(std::remove_if(begin(), end(), f), end());
  }
};


template <class RC>
IMP_OUTPUT_OPERATOR(VectorOfRefCounted<RC>)

template <class RC>
IMP_SWAP(VectorOfRefCounted<RC>);


IMP_END_NAMESPACE

#endif  /* IMP_REF_COUNTED_H */
