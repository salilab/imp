/**
 *  \file spb_representation.h
 *  \brief SPB Representation
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_REPRESENTATION_H
#define IMPSPB_SPB_REPRESENTATION_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT atom::Hierarchies create_representation(
    Model *m, RestraintSet *allrs, spb::SPBParameters mydata,
    container::ListSingletonContainer *CP_ps,
    container::ListSingletonContainer *IL2_ps, core::MonteCarloMovers &mvs,
    Particle *SideXY, Particle *SideZ, int iseed);

IMPSPBEXPORT atom::Molecule create_protein(
    Model *m, RestraintSet *allrs, std::string name, double mass, int nbeads,
    display::Color color, int copy, double kappa, algebra::Vector3D x0,
    bool use_connectivity, int start_residue = 1, int length = -1);

IMPSPBEXPORT atom::Molecule create_protein(
    Model *m, std::string name, std::string filename, int nbeads,
    display::Color colore, int copy, algebra::Vector3D x0, int offset = 0,
    bool makerigid = true);

IMPSPBEXPORT atom::Molecule create_merged_protein(
    Model *m, RestraintSet *allrs, std::string name, atom::Molecules proteins,
    int copy, double kappa, double dist = -1.0);

IMPSPBEXPORT void recenter_rb(core::RigidBody &rb, core::XYZRs &rbps,
                                   algebra::Vector3D x0,
                                   bool coiledcoil = false);

IMPSPBEXPORT atom::Molecules create_coiled_coil(
    Model *m, std::string name, std::string filename_A, std::string filename_B,
    int nbeads, display::Color colore, int copy, algebra::Vector3D x0,
    int offset = 0);

IMPSPBEXPORT void load_restart(atom::Hierarchies &all_mol,
                                    spb::SPBParameters mydata);

IMPSPBEXPORT atom::Molecule create_GFP(
    Model *m, std::string name, int copy,
    container::ListSingletonContainer *lsc, algebra::Vector3D x0,
    core::MonteCarloMovers &mvs, spb::SPBParameters mydata,
    Particle *SideXY, Particle *SideZ);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_REPRESENTATION_H */
