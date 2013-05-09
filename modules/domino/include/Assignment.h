/**
 *  \file IMP/domino/Assignment.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_H
#define IMPDOMINO_ASSIGNMENT_H

#include <IMP/domino/domino_config.h>
#include "domino_macros.h"
#include "Subset.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/base/Value.h>
#include <IMP/base/ConstVector.h>
#include <IMP/macros.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Store a configuration of a subset.
/** This class stores a configuration of a Subset object. The
    indices of the corresponding Subset/Assignment objects
    correspond. That is, the state of the ith particle in
    a Subset is the ith value in the Assignment.

    Like Subset objects, Assignment objects cannot be
    modified and provide a vector/python list like
    interface.
*/
class IMPDOMINOEXPORT Assignment : public base::ConstVector<int> {
  typedef base::ConstVector<int> P;

 public:
  Assignment() {}
  Assignment(unsigned int sz) : P(sz, -1) {}
  template <class It> Assignment(It b, It e) : P(b, e) {}
  explicit Assignment(const Ints &i) : P(i.begin(), i.end()) {}
};

IMP_VALUES(Assignment, Assignments);
IMP_SWAP(Assignment);

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_ASSIGNMENT_H */
