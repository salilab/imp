/**
 *  \file domino/SubsetState.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_STATE_H
#define IMPDOMINO_SUBSET_STATE_H

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
    indices of the corresponding Subset/SubsetState objects
    correspond. That is, the state of the ith particle in
    a Subset is the ith value in the SubsetState.

    Like Subset objects, SubsetState objects cannot be
    modified and provide a std::vector/python list like
    interface.
*/
class SubsetState {
  boost::shared_array<int> v_;
  unsigned int sz_;
  int compare(const SubsetState &o) const {
    IMP_USAGE_CHECK(std::find(o.v_.get(), o.v_.get()+o.sz_, -1)
                    ==  o.v_.get()+o.sz_,
                    "SubsetState not initialize yet.");
    IMP_USAGE_CHECK(std::find(v_.get(), v_.get()+sz_, -1)
                    ==  v_.get()+sz_,
                    "SubsetState not initialize yet.");
    IMP_USAGE_CHECK(o.size() == size(), "Sizes don't match");
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o[i]) return -1;
      else if (v_[i] > o[i]) return 1;
    }
    return 0;
  }
  void validate() const {
    for (unsigned int i=0; i< sz_; ++i) {
      IMP_USAGE_CHECK(v_[i] != -1, "Invalid initialization");
    }
  }
public:
  typedef SubsetState This;
  SubsetState(){}
  template <class It>
  SubsetState(It b, It e): v_(new int[std::distance(b,e)]),
                           sz_(std::distance(b,e)){
    std::copy(b,e, v_.get());
    validate();
  }
  SubsetState(const Ints &i): v_(new int[i.size()]), sz_(i.size()){
    std::copy(i.begin(), i.end(), v_.get());
    validate();
  }
  IMP_COMPARISONS;
#ifndef SWIG
  int operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < sz_, "Out of range");
    IMP_USAGE_CHECK(v_[i] >=0, "Not initialized properly");
    return v_[i];
  }
#endif
#ifndef IMP_DOXYGEN
  int __getitem__(unsigned int i) const {
    if (i >= sz_) IMP_THROW("Out of bound", IndexException);
    return operator[](i);
  }
  unsigned int __len__() const {return sz_;}
#endif
#ifndef SWIG
  unsigned int size() const {
    return sz_;
  }
#endif
  IMP_SHOWABLE_INLINE(SubsetState, {
      out << "[";
      for (unsigned int i=0; i< size(); ++i) {
        out << v_[i];
        if (i != size()-1) out << " ";
      }
      out << "]";
    });
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  typedef const int * const_iterator;
  const_iterator begin() const {
    return v_.get();
  }
  const_iterator end() const {
    return v_.get()+size();
  }
#endif
  IMP_HASHABLE_INLINE(SubsetState, return boost::hash_range(begin(),
                                                       end()););
};

IMP_VALUES(SubsetState, SubsetStates);


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline std::size_t hash_value(const SubsetState &t) {
  return t.__hash__();
}
#endif

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_STATE_H */
