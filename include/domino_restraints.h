/**
 *  \file domino_restraints.h
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_DOMINO_RESTRAINTS_H
#define IMPMEMBRANE_DOMINO_RESTRAINTS_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/atom.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT RestraintSet* create_restraints
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr);

IMPMEMBRANEEXPORT void add_excluded_volume
(Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT void add_DOPE (Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT void add_packing_restraint
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr);

IMPMEMBRANEEXPORT core::PairRestraint* add_distance_restraint
 (Model *m,Particle *s0,Particle *s1,double x0);

IMPMEMBRANEEXPORT void add_x_restraint (Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT void add_y_restraint (Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT void add_depth_restraint
(Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT core::PairRestraint* add_interacting_restraint
(Model *m,Particle *rb0,Particle *rb1,core::TableRefiner *tbr);

IMPMEMBRANEEXPORT void add_diameter_restraint
(Model *m, atom::Hierarchy protein);

IMPMEMBRANEEXPORT void add_tilt_restraint
(Model *m, atom::Hierarchy protein);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_RESTRAINTS_H */
