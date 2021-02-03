/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H
#define IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H

#include <IMP/kernel_config.h>
#include "TupleRestraint.h"
#include "container_helpers.h"
#include "../container_macros.h"
#include <sstream>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Container, class Score>
Restraints create_current_decomposition(Model *m, Score *score,
                                        Container *c, std::string name) {
  IMP_CHECK_VARIABLE(m);
  IMP_USAGE_CHECK(m, "nullptr passed for the Model.");
  IMP_USAGE_CHECK(score, "nullptr passed for the Score.");
  Restraints ret;
  IMP_FOREACH(typename Container::ContainedIndexType i, c->get_contents()) {
    double cscore = score->evaluate_index(m, i, nullptr);
    if (cscore != 0) {
      std::ostringstream oss;
      oss << name << " " << Showable(i);
      Pointer<Restraint> r =
          IMP::internal::create_tuple_restraint(score, m, i, oss.str());
      r->set_last_score(cscore);
      ret.push_back(r);
    }
  }
  return ret;
}

template <class Container, class Score>
Restraints create_decomposition(Model *m, Score *score, Container *c,
                                std::string name) {
  IMP_USAGE_CHECK(m, "nullptr passed for the Model.");
  IMP_USAGE_CHECK(score, "nullptr passed for the Score.");
  typename Container::ContainedIndexTypes all = c->get_range_indexes();
  Restraints ret(all.size());
  for (unsigned int i = 0; i < all.size(); ++i) {
    std::ostringstream oss;
    oss << name << " " << Showable(all[i]);
    ret[i] = IMP::internal::create_tuple_restraint(score, m, all[i],
                                                           oss.str());
  }
  return ret;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H */
