/**
 *  \file spb_test.h
 *  \brief Assemble SPB restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_TEST_H
#define IMPMEMBRANE_SPB_TEST_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/membrane/spb_main.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT void spb_assemble_restraints
 (Model *m, membrane::SPBParameters myparam, atom::Hierarchies& all_mol,
 container::ListSingletonContainer *bCP_ps,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_TEST_H */
