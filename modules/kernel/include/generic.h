/**
 *  \file IMP/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_GENERIC_H
#define IMPKERNEL_GENERIC_H

#include "macros.h"
#include "kernel_config.h"
#include "internal/scoring_functions.h"
#include "internal/generic_impl.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

IMP_BEGIN_NAMESPACE



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
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Score>
inline Restraint* create_restraint(Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  if (name==std::string()) {
      name= std::string("Restraint on ")+s->get_name();
  }
  return new internal::TupleRestraint<Score>(s, t, name);
}

template <class Score>
inline Restraint* create_restraint(const Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
    if (name==std::string()) {
      std::ostringstream oss;
      oss << s->get_name() << " on " << t;
      name= oss.str();
  }
    return new internal::TupleRestraint<Score>(const_cast<Score*>(s), t, name);
}



/** When programming in C++, you can use generic constraint instead
    of a SingletonConstraint, PairConstraint, etc. The result is
    somewhat faster (20% or so). See
    \ref cppspecialization "specializing for speed" for more
    information.
*/
template <class Before, class After>
inline Constraint* create_constraint(Before *b, After *a,
                              const typename Before::Argument &t,
                              std::string name=std::string()) {
  if (name==std::string()) {
    if (b) name+= " and  "+b->get_name();
    if (a) name+= " and " +a->get_name();
  }
  return new internal::TupleConstraint<Before, After>(b, a, t, name);
}


IMP_END_NAMESPACE

#include "internal/generic_impl.h"

#endif  /* IMPKERNEL_GENERIC_H */
