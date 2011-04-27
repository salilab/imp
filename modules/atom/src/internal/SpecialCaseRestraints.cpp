/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */


#include <IMP/atom/internal/SpecialCaseRestraints.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE
SpecialCaseRestraints::SpecialCaseRestraints(Model *m,
                                             const ParticlesTemp &ps): ps_(ps) {
  SetLogState ss(SILENT); // don't print all the dep graph crap
  RestraintsTemp rs= get_restraints(m->get_root_restraint_set());
  dg_=get_dependency_graph(rs);
}

IMPATOM_END_INTERNAL_NAMESPACE
