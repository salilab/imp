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
(Model *m, membrane::SPBParameters mydata,
container::ListSingletonContainer *CP_ps,
container::ListSingletonContainer *IL2_ps,
core::Movers& mvs,
Particle *SideXY, Particle *SideZ, int iseed);

IMPMEMBRANEEXPORT atom::Molecule create_protein
(Model *m,std::string name,double mass,int nbeads,display::Color color,
int copy,double kappa,algebra::Vector3D x0,
bool use_connectivity,int start_residue=1,int length=-1);

IMPMEMBRANEEXPORT atom::Molecule create_protein
(Model *m,std::string name,std::string filename,int nbeads,
 display::Color colore,int copy,algebra::Vector3D x0,
 int offset=0, bool makerigid=true);

IMPMEMBRANEEXPORT atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecules proteins,
 int copy,double kappa,double dist=-1.0);

IMPMEMBRANEEXPORT void recenter_rb(core::RigidBody& rb,core::XYZRs& rbps,
 algebra::Vector3D x0, double flip=1.0);

IMPMEMBRANEEXPORT atom::Molecules create_coiled_coil
(Model *m,std::string name,std::string filename_A,std::string filename_B,
int nbeads,display::Color colore,int copy,
algebra::Vector3D x0,int offset=0);

IMPMEMBRANEEXPORT void load_restart
 (atom::Hierarchies& all_mol, membrane::SPBParameters mydata);

IMPMEMBRANEEXPORT atom::Molecule create_GFP
 (Model *m, std::string name, int copy,
 container::ListSingletonContainer *lsc, algebra::Vector3D x0,
 core::Movers& mvs, membrane::SPBParameters mydata,
 Particle *SideXY, Particle *SideZ);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_REPRESENTATION_H */
