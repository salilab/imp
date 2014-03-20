/**
 *  \file IMP/domino/Order.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ORDER_H
#define IMPDOMINO_ORDER_H

#include <IMP/domino/domino_config.h>
#include "IMP/macros.h"
#include "domino_macros.h"
#include "Subset.h"
#include "Assignment.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/base/Value.h>
#include <IMP/base/ConstVector.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Store a persistent ordering for a subset based on the list.
/**  Return a list of indexes into s, representing a permutation of the
    particles in s, so that they are ordered according to all_particles.
    This order can be used to write s to disk, as the order in s can
    change between domino runs.
*/
class IMPDOMINOEXPORT Order : public base::ConstVector<unsigned int> {
  typedef base::ConstVector<unsigned int> P;

 public:
  Order() {}
  Order(Subset outer, const kernel::ParticlesTemp &order);
  template <class It>
  Ints get_list_ordered(It b, It e) const {
    IMP_USAGE_CHECK(static_cast<unsigned int>(std::distance(b, e)) == size(),
                    "Sizes don't match in permutation");
    Ints ret(std::distance(b, e));
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[operator[](i)] = b[i];
    }
    IMP_USAGE_CHECK(get_subset_ordered(ret) == Assignment(b, e),
                    "In and out don't match: " << Assignment(b, e) << " vs "
                                               << get_subset_ordered(ret));
    return ret;
  }
  template <class It>
  Assignment get_subset_ordered(It b, It e) const {
    IMP_CHECK_VARIABLE(e);
    IMP_USAGE_CHECK(static_cast<unsigned int>(std::distance(b, e)) == size(),
                    "Sizes don't match in permutation");
    Assignment ret(size());
    for (unsigned int i = 0; i < size(); ++i) {
      ret.set_item(i, b[operator[](i)]);
    }
    return ret;
  }
  Ints get_list_ordered(const Assignment &a) const {
    return get_list_ordered(a.begin(), a.end());
  }
  Assignment get_subset_ordered(const Ints &a) const {
    return get_subset_ordered(a.begin(), a.end());
  }
};

IMP_VALUES(Order, Orders);
IMP_SWAP(Order);

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_ORDER_H */
