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
/** The interface of this class is like that of std::vector except
    you must use VectorOfRefCounted::set() to change existing members
    instead of VectorOfRefCounted::operator[]().

    Documentation for std::vector can be found at as part of the SGI
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
  typedef RC& reference;
  VectorOfRefCounted(const std::vector<RC> &o):data_(o) {
    ref(o.begin(), o.end());
  }
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

  operator const std::vector<RC>&() const {
    return data_;
  }
#endif
  RC operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < size(), "Index out of range in []: "
              << i << ">=" << size(), IndexException);
    return data_[i];
  }
  void set(unsigned int i, RC p) {
    IMP_USAGE_CHECK(i < size(), "Index out of range in set "
              << i << ">=" << size(), IndexException);
    unref(data_[i]);
    data_[i]=p;
    ref(p);
  }
  RC back() const {
    IMP_USAGE_CHECK(!empty(), "Can't call back on empty container",
              InvalidStateException);
    return data_.back();
  }
  RC front() const {
    IMP_USAGE_CHECK(!empty(), "Can't call front on empty container",
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
#ifndef SWIG
  const_iterator begin() const {return data_.begin();}
  const_iterator end() const {return data_.end();}
#endif
  iterator begin() {return data_.begin();}
  iterator end() {return data_.end();}
  template <class It>
  void insert(iterator loc, It b, It e) {
    data_.insert(data_.begin()+(loc-data_.begin()), b, e);
    ref(b,e);
  }
  void insert(iterator loc, RC p) {
    data_.insert(loc, p);
    ref(p);
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
  void swap_with(std::vector<RC> &a) {
    std::swap(a, data_);
    ref(data_.begin(), data_.end());
    unref(a.begin(), a.end());
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
    std::vector<RC> bye;
    for (iterator it= begin(); it != end(); ++it) {
      if (f(*it)) bye.push_back(*it);
    }
    if (!bye.empty()) {
      data_.resize(std::remove_if(begin(), end(), f)-begin());
      unref(bye.begin(), bye.end());
    }
  }
  void remove(RC r) {
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (data_[i]==r) {
        unref(r);
        data_.erase(data_.begin()+i);
      }
    }
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

template <class RC, class Policy>
void swap(VectorOfRefCounted<RC, Policy> &a,
          std::vector<RC> &b) {
  a.swap_with(b);
}
template <class RC, class Policy>
void swap(std::vector<RC> &b,
          VectorOfRefCounted<RC, Policy> &a) {
  a.swap_with(b);
}
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {

template <class T, class F>
void remove_if(T &t, const F &f) {
  t.remove_if(f);
}

template <class T, class F>
void remove_if(std::vector<T> &t, const F &f) {
  t.erase(std::remove_if(t.begin(), t.end(), f), t.end());
}



}
#endif

IMP_END_NAMESPACE

#endif  /* IMP_VECTOR_OF_REF_COUNTED_H */
