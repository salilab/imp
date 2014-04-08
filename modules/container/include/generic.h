/**
 *  \file IMP/container/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_GENERIC_H
#define IMPCONTAINER_GENERIC_H

#include <IMP/container/container_config.h>
#include <IMP/kernel/internal/ContainerRestraint.h>
#include <IMP/kernel/internal/ContainerConstraint.h>
#include "InContainerPairFilter.h"

IMPCONTAINER_BEGIN_NAMESPACE

/** Create a restraint from a score and a container. The resulting restraint
    is, in general, more efficient than creating an, eg, PairsRestraint.

    A python version of this is provided, but it produces a slightly less
    efficient restraint.
*/
template <class Score, class Container>
inline kernel::Restraint *create_restraint(Score *s, Container *c,
                                           std::string name = std::string()) {
  return IMP::internal::create_container_restraint(s, c, name);
}

/** Helper to create a ContainerConstraint.
 */
template <class Container, class Before, class After>
inline Constraint *create_constraint(Before *b, After *a, Container *c,
                                     std::string name = std::string()) {
  return IMP::internal::create_container_constraint(c, b, a, name);
}

/** Create a filter that returns whether the pair, or its flip is
    contained in the passed container.

    \note This is different than the InContainerPairFilter.*/
template <class Container>
inline PairPredicate *create_in_container_filter(Container *c,
                                                 std::string name =
                                                     std::string()) {
  return new InContainerPairFilter(c, name);
}

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_GENERIC_H */
