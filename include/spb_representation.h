/**
 *  \file spb_representation.h
 *  \brief SPB Representation
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_REPRESENTATION_H
#define IMPMEMBRANE_SPB_REPRESENTATION_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT atom::Hierarchies create_representation
(Model *m, SPBParameters mydata,
container::ListSingletonContainer *CP_ps,
container::ListSingletonContainer *IL2_ps, core::Movers& mvs);

IMPMEMBRANEEXPORT atom::Molecule create_protein
(Model *m,std::string name,double mass,int nbeads,display::Color color,
int copy,double kappa,algebra::Vector3D x0,
int start_residue=1,int length=-1);

IMPMEMBRANEEXPORT atom::Molecule create_protein
(Model *m,std::string name,std::string filename,int nbeads,
 display::Color colore,int copy,algebra::Vector3D x0,
 int start_residue=1,bool recenter=true);

IMPMEMBRANEEXPORT atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecules proteins,
 int copy,double kappa,double dist=-1.0);

IMPMEMBRANEEXPORT atom::Molecules create_coiled_coil
(Model *m,std::string name,std::string filename_A, std::string filename_B,
int nbeads,display::Color colore,int copy,
algebra::Vector3D x0,int start_residue);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_REPRESENTATION_H */
