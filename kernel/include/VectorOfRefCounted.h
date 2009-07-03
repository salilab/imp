/**
 *  \file VectorOfRefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_VECTOR_OF_REF_COUNTED_H
#define IMP_VECTOR_OF_REF_COUNTED_H

#include "RefCounted.h"

#include <vector>

IMP_BEGIN_NAMESPACE

//! A vector-like container for reference counted objects
/** Documentation for std::vector can be found at as part of the SGI
    stl documentation, among other places
    (http://www.sgi.com/tech/stl/Vector.html).

    The first template argument should, in general, be a pointer to the
    type of object. The second tells the container how to do ref counting
    and should probably be left with its default value.

    When used within Python, IMP::VectorOfRefCounted acts like a Python list.
 */
template <class RC, class Policy= IMP::RefCounted::Policy>
class VectorOfRefCounted {
  typedef std::vector<RC> Data;
  Data data_;
  // make sure they are converted to RC
  void ref(RC v) {
    Policy::ref(v);
  }
  void unref(RC v) {
    Policy::unref(v);
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
  VectorOfRefCounted(const VectorOfRefCounted<RC, Policy> &o):
    data_(o.begin(), o.end()){
    ref(o.begin(), o.end());
  }
  VectorOfRefCounted(RC rc): data_(1, rc) {
    ref(rc);
  }
  VectorOfRefCounted(unsigned int i): data_(i, RC()){}
  VectorOfRefCounted(){}
  ~VectorOfRefCounted() {
    clear();
  }
#ifndef SWIG
  const VectorOfRefCounted<RC, Policy>
  operator=(const VectorOfRefCounted<RC, Policy> &o) {
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
    unref(data_[i]);
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
  void resize(unsigned int i) {data_.resize(i);}
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
  void swap_with(VectorOfRefCounted<RC, Policy> &a) {
    std::swap(a.data_, data_);
  }
#endif
  void clear(){
    unref(data_.begin(), data_.end());
    data_.clear();
  }
  /*void show(std::ostream &out) const {
    for (unsigned int i=0; i< size(); ++i) {
      if (operator[](i)) {
        out << operator[](i)->get_name() << " ";
      } else {
        out << "NULL ";
      }
    }
    }*/
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

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
/*template <class RC, class Policy>
std::ostream &operator<<(std::ostream &out,
                         const VectorOfRefCounted<RC, Policy> &v) {
  v.show(out);
  return out;
  }*/

template <class RC, class Policy>
void swap(VectorOfRefCounted<RC, Policy> &a,
          VectorOfRefCounted<RC, Policy> &b) {
  a.swap_with(b);
}
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
template <class T, class F>
void remove_if(VectorOfRefCounted<T> &t, const F &f) {
  t.remove_if(f);
}
}
#endif

IMP_END_NAMESPACE

#endif  /* IMP_VECTOR_OF_REF_COUNTED_H */
