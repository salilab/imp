/**
 *  \file spb_test.h
 *  \brief Assemble SPB restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_ASSEMBLE_RESTRAINTS_H
#define IMPMEMBRANE_SPB_ASSEMBLE_RESTRAINTS_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/membrane/spb_main.h>
#include <map>
#include <string>
#include "membrane_config.h"

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT std::map<std::string, IMP::Pointer<RestraintSet> >
spb_assemble_restraints(Model *m, RestraintSet *allrs,
                        membrane::SPBParameters &myparam,
                        atom::Hierarchies &all_mol,
                        container::ListSingletonContainer *CP_ps,
                        container::ListSingletonContainer *IL2_ps,
                        std::map<std::string, IMP::Pointer<Particle> > ISD_ps);

IMPMEMBRANE_END_NAMESPACE

#endif /* IMPMEMBRANE_SPB_ASSEMBLE_RESTRAINTS_H */
