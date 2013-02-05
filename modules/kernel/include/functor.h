/**
 *  \file IMP/kernel/functor.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FUNCTOR_H
#define IMPKERNEL_FUNCTOR_H

#include <IMP/kernel/kernel_config.h>
#include "internal/functors.h"

IMPKERNEL_BEGIN_NAMESPACE

/** Return a functor that returns true when the predicate has a certain
    value.*/
template <class Pred>
inline internal::PredicateEquals<Pred, true>
make_predicate_equal(const Pred *p,
                     Model *m,
                     int value) {
  return internal::PredicateEquals<Pred, true>(p, m, value);
}

/** Return a functor that returns true when the predicate doesn't have a certain
    value.*/
template <class Pred>
inline internal::PredicateEquals<Pred, false>
make_predicate_not_equal(const Pred *p,
                         Model *m,
                         int value) {
  return internal::PredicateEquals<Pred, false>(p, m, value);
}

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_FUNCTOR_H */
