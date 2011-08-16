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
#include "domino_macros.h"
#include "Subset.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/Pointer.h>


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
  boost::scoped_array<int> v_;
  unsigned int sz_;
  int compare(const Assignment &o) const {
    IMP_USAGE_CHECK(std::find(o.begin(), o.end(), -1)
                    ==  o.end(),
                    "Assignment not initialize yet.");
    IMP_USAGE_CHECK(std::find(begin(), end(), -1) == end(),
                    "Assignment not initialize yet.");
    IMP_USAGE_CHECK(o.size() == size(), "Sizes don't match");
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o.v_[i]) return -1;
      else if (v_[i] > o.v_[i]) return 1;
    }
    return 0;
  }
  void validate() const {
    if (v_==NULL) {
      IMP_USAGE_CHECK(sz_==0, "NULL but not uninitialized");
    } else {
      for (unsigned int i=0; i< sz_; ++i) {
        IMP_USAGE_CHECK(v_[i] != -1, "Invalid initialization");
      }
    }
  }
  void create(unsigned int sz) {
    IMP_USAGE_CHECK(sz>0, "can't create 0 size subset");
    v_.reset(new int[sz]);
    sz_=sz;
  }
  void copy_from(const Assignment &o) {
    validate();
    o.validate();
    sz_=o.sz_;
    v_.reset(new int[sz_]);
    std::copy(o.v_.get(), o.v_.get()+sz_, v_.get());
  }
public:
  ~Assignment() {
  }
  Assignment(): v_(0), sz_(0){}
  template <class It>
  Assignment(It b, It e) {
    IMP_USAGE_CHECK(std::distance(b,e) > 0,
                    "Can't create Assignment from empty list");
    create(std::distance(b,e));
    std::copy(b,e, v_.get());
    validate();
  }
  Assignment(const Ints &i) {
    IMP_USAGE_CHECK(!i.empty(),
                    "Can't create Assignment from empty list");
    create(i.size());
    std::copy(i.begin(), i.end(), v_.get());
    validate();
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Assignment(const Assignment &o): sz_(0) {
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
    return v_[i];
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
      out << "(";
      for (unsigned int i=0; i< size(); ++i) {
        out << v_[i];
        if (i != size()-1) out << " ";
      }
      out << ")";
    });
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  typedef const int * const_iterator;
  const_iterator begin() const {
    return v_.get();
  }
  const_iterator end() const {
    return v_.get()+size();
  }
  void swap_with(Assignment &o) {
    std::swap(sz_, o.sz_);
    v_.swap(o.v_);
  }
#endif
  IMP_HASHABLE_INLINE(Assignment, return boost::hash_range(begin(),
                                                       end()););
};

IMP_VALUES(Assignment, Assignments);
IMP_SWAP(Assignment);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline std::size_t hash_value(const Assignment &t) {
  return t.__hash__();
}
#endif

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_ASSIGNMENT_H */
