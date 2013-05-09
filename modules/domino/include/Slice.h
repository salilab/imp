/**
 *  \file IMP/domino/Slice.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SLICE_H
#define IMPDOMINO_SLICE_H

#include <IMP/domino/domino_config.h>
#include "IMP/macros.h"
#include "domino_macros.h"
#include "Subset.h"
#include "Assignment.h"
#include <IMP/base/Value.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/base/ConstVector.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Store a subset of a subset or assignment.
/** This class stores a particular slice through a subset. The entire
    inner Subset must be contained in the outer one.
*/
class IMPDOMINOEXPORT Slice : public base::ConstVector<unsigned int> {
  typedef base::ConstVector<unsigned int> P;
  static Ints get_slice(Subset outer, Subset inner) {
    Ints ret(inner.size());
    for (unsigned int i = 0; i < inner.size(); ++i) {
      for (unsigned int j = 0; j < outer.size(); ++j) {
        if (inner[i] == outer[j]) {
          ret[i] = j;
        }
      }
    }
    return ret;
  }

 public:
  Slice() {}
  Slice(Subset outer, Subset inner) : P(get_slice(outer, inner)) {}
  Assignment get_sliced(const Assignment& a) const {
    Ints ret(size(), -1);
    for (unsigned int i = 0; i < size(); ++i) {
      ret[i] = a[operator[](i)];
    }
    return Assignment(ret);
  }
  Subset get_sliced(const Subset& a) const {
    ParticlesTemp ret(size());
    for (unsigned int i = 0; i < size(); ++i) {
      ret[i] = a[operator[](i)];
    }
    return Subset(ret, true);
  }
};

IMP_VALUES(Slice, Slices);
IMP_SWAP(Slice);

/** Return a slice for the inner subset if it is not contained
    in any of the excluded subsets, otherwise return the empty slice.
*/
inline Slice get_slice(Subset outer, Subset inner, const Subsets& excluded) {
  IMP_USAGE_CHECK(inner.size() <= outer.size(), "Inner and outer are switched");
  for (unsigned int i = 0; i < excluded.size(); ++i) {
    if (get_intersection(inner, excluded[i]).size() == inner.size()) {
      return Slice();
    }
  }
  return Slice(outer, inner);
}

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_SLICE_H */
