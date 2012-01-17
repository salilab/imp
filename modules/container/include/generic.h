/**
 *  \file container/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_GENERIC_H
#define IMPCONTAINER_GENERIC_H

#include "container_config.h"
#include <IMP/core/internal/generic.h>

IMPCONTAINER_BEGIN_NAMESPACE


/** Create a restraint from a score and a container. The resulting restraint
    is, in general, more efficient than creating an, eg, PairsRestraint.
    \relatesalso ContainerRestraint
*/
template <class Score, class Container>
inline Restraint *create_restraint(Score *s, Container*c,
                            std::string name=std::string()) {
  return core::internal::create_restraint(s, c, name);
}



/** \relatesalso ContainerConstraint

    Helper to create a ContainerConstraint.
 */
template <class Container, class Before, class After>
inline Constraint *create_constraint(Before *b, After *a, Container *c,
                              std::string name=std::string()) {
  return core::internal::create_constraint(c, b, a, name);
}


/** Create a filter that returns whether the pair, or its flip is
    contained in the passed container.*/
template <class Container>
inline PairFilter*
create_in_container_filter(Container *c, std::string name=std::string()) {
  return core::internal::create_in_container_filter(c, name);
}




IMPCONTAINER_END_NAMESPACE


#endif  /* IMPCONTAINER_GENERIC_H */
