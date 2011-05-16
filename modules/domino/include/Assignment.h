/**
 *  \file domino/Assignment.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_H
#define IMPDOMINO_ASSIGNMENT_H

#include "domino_config.h"
#include "IMP/macros.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/Pointer.h>
#include <boost/shared_array.hpp>
#include <boost/functional/hash.hpp>
#include <algorithm>
#include <boost/functional/hash/hash.hpp>

IMPDOMINO_BEGIN_NAMESPACE

//! Store a configuration of a subset.
/** This class stores a configuration of a Subset object. The
    indices of the corresponding Subset/Assignment objects
    correspond. That is, the state of the ith particle in
    a Subset is the ith value in the Assignment.

    Like Subset objects, Assignment objects cannot be
    modified and provide a std::vector/python list like
    interface.
*/
class IMPDOMINOEXPORT Assignment {
  // store the ref count in the first entry of the block
  static std::vector<std::vector<int*> > free_;
  int* v_;
  unsigned int sz_;
  int compare(const Assignment &o) const {
    IMP_USAGE_CHECK(std::find(o.begin(), o.end(), -1)
                    ==  o.end(),
                    "Assignment not initialize yet.");
    IMP_USAGE_CHECK(std::find(begin(), end(), -1) == end(),
                    "Assignment not initialize yet.");
    IMP_USAGE_CHECK(o.size() == size(), "Sizes don't match");
    for (unsigned int i=1; i< size()+1; ++i) {
      if (v_[i] < o.v_[i]) return -1;
      else if (v_[i] > o.v_[i]) return 1;
    }
    return 0;
  }
  void validate() const {
    for (unsigned int i=0; i< sz_; ++i) {
      IMP_USAGE_CHECK(v_[i+1] != -1, "Invalid initialization");
    }
  }
  void teardown() {
    if (sz_ >0) {
      --v_[0];
      IMP_USAGE_CHECK(v_[0] < 10000,
                      "Bad ref count: " << v_[0]);
      if (v_[0]==0) {
        free_[sz_-1].push_back(v_);
      }
    }
  }
  void create(unsigned int sz) {
    IMP_USAGE_CHECK(sz>0, "can't create 0 size subset");
    if (free_.size() < sz) {
      free_.resize(sz);
    }
    if (free_[sz-1].empty()) {
      v_=new int[sz+1];
    } else {
      v_=free_[sz-1].back();
      free_[sz-1].pop_back();
    }
    v_[0]=1;
    sz_=sz;
  }
  void copy_from(const Assignment &o) {
    validate();
    o.validate();
    teardown();
    v_=o.v_;
    sz_=o.sz_;
    if (sz_ >0) {
      ++v_[0];
    }
  }
public:
  ~Assignment() {
    teardown();
  }
  Assignment(): v_(0), sz_(0){}
  template <class It>
  Assignment(It b, It e) {
    IMP_USAGE_CHECK(std::distance(b,e) > 0,
                    "Can't create Assignment from empty list");
    create(std::distance(b,e));
    std::copy(b,e, v_+1);
    validate();
  }
  Assignment(const Ints &i) {
    IMP_USAGE_CHECK(!i.empty(),
                    "Can't create Assignment from empty list");
    create(i.size());
    std::copy(i.begin(), i.end(), v_+1);
    validate();
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Assignment(const Assignment &o):v_(0), sz_(0){
    copy_from(o);
  }
  Assignment& operator=(const Assignment &o) {
    copy_from(o);
    return *this;
  }
#endif
  IMP_COMPARISONS(Assignment);
#ifndef SWIG
  int operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < sz_, "Out of range");
    IMP_USAGE_CHECK(v_[i+1] >=0, "Not initialized properly");
    return v_[i+1];
  }
#endif
#ifndef IMP_DOXYGEN
  int __getitem__(unsigned int i) const {
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
  IMP_SHOWABLE_INLINE(Assignment, {
      out << "[";
      for (unsigned int i=0; i< size(); ++i) {
        out << v_[i+1];
        if (i != size()-1) out << " ";
      }
      out << "]";
    });
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  typedef const int * const_iterator;
  const_iterator begin() const {
    return v_+1;
  }
  const_iterator end() const {
    return v_+1+size();
  }
#endif
  IMP_HASHABLE_INLINE(Assignment, return boost::hash_range(begin(),
                                                       end()););
};

IMP_VALUES(Assignment, Assignments);


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline std::size_t hash_value(const Assignment &t) {
  return t.__hash__();
}
#endif

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_ASSIGNMENT_H */
