/**
 *  \file spb_create_restraints.h
 *  \brief SPB create restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_CREATE_RESTRAINTS_H
#define IMPMEMBRANE_SPB_CREATE_RESTRAINTS_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/container.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT void spb_assemble_restraints
(Model *m, SPBParameters mydata, atom::Hierarchies& all_mol,
 container::ListSingletonContainer *bCP_ps,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_CREATE_RESTRAINTS_H */
