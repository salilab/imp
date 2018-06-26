/**
 *  \file IMP/integrative_docking/helpers.h   \brief A set of helper functions
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_HELPERS_H
#define IMPINTEGRATIVE_DOCKING_HELPERS_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/algebra/constants.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

#include <vector>
#include <string>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

IMPINTEGRATIVEDOCKINGEXPORT
IMP::atom::Hierarchy read_pdb(const std::string pdb_file_name,
                              IMP::Model* model,
                              IMP::ParticleIndexes& pis);

IMPINTEGRATIVEDOCKINGEXPORT
IMP::ParticlesTemp add_bonds(IMP::atom::Hierarchy mhd);

IMPINTEGRATIVEDOCKINGEXPORT
void read_pdb_atoms(const std::string file_name,
                    IMP::Particles& particles);

IMPINTEGRATIVEDOCKINGEXPORT
void read_pdb_ca_atoms(const std::string file_name,
                       IMP::Particles& particles);

IMPINTEGRATIVEDOCKINGEXPORT
void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms);

IMPINTEGRATIVEDOCKINGEXPORT
void transform(IMP::Particles& ps, IMP::algebra::Transformation3D& t);

IMPINTEGRATIVEDOCKINGEXPORT
void transform(IMP::Model* model, IMP::ParticleIndexes& pis,
               const IMP::algebra::Transformation3D& t);

IMPINTEGRATIVEDOCKINGEXPORT
void get_atom_2_residue_map(const IMP::Particles& atom_particles,
                            const IMP::Particles& residue_particles,
                            std::vector<int>& atom_2_residue_map);

IMPINTEGRATIVEDOCKINGEXPORT
IMP::algebra::Vector3D get_ca_coordinate(const IMP::Particles& ca_atoms,
                                         int residue_index,
                                         std::string chain_id);

IMPINTEGRATIVEDOCKINGEXPORT
void get_residue_solvent_accessibility(
    const IMP::Particles& residue_particles,
    IMP::Floats& residue_solvent_accessibility);

IMPINTEGRATIVEDOCKINGEXPORT
void get_residue_solvent_accessibility(
    const IMP::Particles& atom_particles,
    const IMP::Particles& residue_particles,
    const std::vector<int>& atom_2_residue_map,
    std::vector<float>& residue_solvent_accessibility);

IMPINTEGRATIVEDOCKINGEXPORT
IMP::atom::Residue find_residue(const IMP::ParticlesTemp& residues,
                                int res_index, std::string chain);

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_HELPERS_H */
