/**
 *  \file IMP/spb/spb_assemble_restraints.h
 *  \brief Assemble SPB restraints
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_ASSEMBLE_RESTRAINTS_H
#define IMPSPB_SPB_ASSEMBLE_RESTRAINTS_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/spb/spb_main.h>
#include <map>
#include <string>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT std::map<std::string, IMP::Pointer<RestraintSet> >
spb_assemble_restraints(Model *m, RestraintSet *allrs,
                        spb::SPBParameters &myparam,
                        atom::Hierarchies &all_mol,
                        container::ListSingletonContainer *CP_ps,
                        container::ListSingletonContainer *IL2_ps,
                        std::map<std::string, IMP::Pointer<Particle> > ISD_ps);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_ASSEMBLE_RESTRAINTS_H */
