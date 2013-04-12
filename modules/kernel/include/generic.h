/**
 *  \file IMP/kernel/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_GENERIC_H
#define IMPKERNEL_GENERIC_H

#include <IMP/kernel/kernel_config.h>
#include "internal/scoring_functions.h"
#include "internal/TupleRestraint.h"
#include "internal/TupleConstraint.h"

IMPKERNEL_BEGIN_NAMESPACE



/** Create a ScoringFunction on a single restraints. This can be faster than
    using a RestraintsScoringFunction.*/
template <class RestraintType>
inline ScoringFunction* create_scoring_function(RestraintType* rs,
                                               double weight=1.0,
                                                double max=NO_MAX,
                                                std::string name
                                                = std::string()) {
  return internal::create_scoring_function(rs, weight, max, name);
}

/** When programming in C++, you can use generic restraint instead
    of a SingletonRestraint, PairRestraint, etc. The result is
    somewhat faster (20% or so).
*/
template <class Score>
inline Restraint* create_restraint(Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  return internal::create_tuple_restraint(s,
                                          IMP::kernel::internal::get_model(t),
                                          IMP::kernel::internal::get_index(t),
                                          name);
}

template <class Score>
inline Restraint* create_restraint(const Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
    return internal::create_tuple_restraint(const_cast<Score*>(s),
                                            IMP::kernel::internal::get_model(t),
                                            IMP::kernel::internal::get_index(t),
                                            name);
}



/** When programming in C++, you can use generic constraint instead
    of a SingletonConstraint, PairConstraint, etc. The result is
    somewhat faster (20% or so).
*/
template <class Before, class After>
inline Constraint* create_constraint(Before *b, After *a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  return internal::create_tuple_constraint(b,a,t, name);
}


IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_GENERIC_H */
