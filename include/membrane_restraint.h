/**
 *  \file  membrane_restraint.h
 *  \brief Membrane stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MEMBRANE_RESTRAINT_H
#define IMPMEMBRANE_MEMBRANE_RESTRAINT_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/atom.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT void create_restraints
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr,Parameters* myparam);

IMPMEMBRANEEXPORT void add_excluded_volume
(Model *m,atom::Hierarchy protein,double kappa);

IMPMEMBRANEEXPORT void add_DOPE
(Model *m,atom::Hierarchy protein,std::string sname);

IMPMEMBRANEEXPORT void add_packing_restraint
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr,
 HelixData *TM,double kappa);

IMPMEMBRANEEXPORT core::PairRestraint* add_distance_restraint
(Model *m,Particle *p0,Particle *p1,double x0,double kappa);

IMPMEMBRANEEXPORT void add_depth_restraint
(Model *m,Particle *p,FloatRange zrange,double kappa);

IMPMEMBRANEEXPORT core::PairRestraint* add_interacting_restraint
(Model *m,Particle *rb0,Particle *rb1,
 core::TableRefiner *tbr,double dist,double kappa);

IMPMEMBRANEEXPORT void add_diameter_restraint
(Model *m,atom::Hierarchy protein,double diameter,
 HelixData *TM,double kappa);

IMPMEMBRANEEXPORT void add_tilt_restraint
(Model *m,Particle *p,FloatRange trange,double kappa);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MEMBRANE_RESTRAINT_H */
