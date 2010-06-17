/**
 *  \file domino2/SubsetState.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_STATE_H
#define IMPDOMINO2_SUBSET_STATE_H

#include "domino2_config.h"
#include "IMP/macros.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/Pointer.h>
#include <boost/shared_array.hpp>
#include <algorithm>

IMPDOMINO2_BEGIN_NAMESPACE


/** Store the configuration of a subset. The indexes match those
    of the particles in the corresponding Subset object. The
    SubsetState should not be copied until after it has been
    completely initialized (all positions set).*/
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
  /*void __setitem__(unsigned int i, unsigned int v) {
    if (i >= sz_) IMP_THROW("Out of bound", IndexException);
    operator[](i)=v;
    }*/
  unsigned int __len__() const {return sz_;}
#endif
  unsigned int size() const {
    return sz_;
  }
  IMP_SHOWABLE_INLINE({
      out << "[";
      for (unsigned int i=0; i< size(); ++i) {
        out << v_[i];
        if (i != size()-1) out << " ";
      }
      out << "]";
    });
};

IMP_VALUES(SubsetState, SubsetStatesList);

IMP_OUTPUT_OPERATOR(SubsetState);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_STATE_H */
