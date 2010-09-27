/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#include <IMP/core/internal/SpecialCaseRestraints.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE
SpecialCaseRestraints::SpecialCaseRestraints(Model *m,
                                             const ParticlesTemp &ps): ps_(ps) {
  RestraintsTemp rs= get_restraints(m->get_root_restraint_set());
  dg_=get_dependency_graph(rs);
}

IMPCORE_END_INTERNAL_NAMESPACE
