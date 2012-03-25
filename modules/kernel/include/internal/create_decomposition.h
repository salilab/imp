/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H
#define IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H

#include "../kernel_config.h"
#include "TupleRestraint.h"
#include "container_helpers.h"
#include <sstream>

IMP_BEGIN_INTERNAL_NAMESPACE


template <class List, class Score>
Restraints create_decomposition(Model *m,Score *score,
                                const List &indexes,
                                std::string name) {
  IMP_USAGE_CHECK(m, "nullptr passed for the Model.");
  IMP_USAGE_CHECK(score, "nullptr passed for the Score.");
  Restraints ret(indexes.size());
  for (unsigned int i=0; i< indexes.size(); ++i) {
    std::ostringstream oss;
    oss << name << " " << IMP::internal::get_particle(m, indexes[i]);
    ret[i]= IMP::internal::create_restraint(score,
                                            IMP::internal::get_particle(m,
                                                                 indexes[i]),
                             oss.str());
  }
  return ret;
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_CREATE_DECOMPOSITION_H */
