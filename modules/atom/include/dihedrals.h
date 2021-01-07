/**
 *  \file IMP/atom/dihedrals.h
 *  \brief Helpers to extract dihedral information.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DIHEDRALS_H
#define IMPATOM_DIHEDRALS_H

#include <IMP/atom/atom_config.h>
#include "Atom.h"
#include "Residue.h"

IMPATOM_BEGIN_NAMESPACE

//IMP_BUILTIN_VALUES(AtomTypes, AtomTypesList);

//! Return the atoms comprising the phi dihedral.
/** If all atoms cannot be found, an empty list is returned.
    \see Residue */
IMPATOMEXPORT Atoms get_phi_dihedral_atoms(Residue rd);

//! Return the atoms comprising the psi dihedral.
/** If all atoms cannot be found, an empty list is returned.
    \see Residue */
IMPATOMEXPORT Atoms get_psi_dihedral_atoms(Residue rd);

//! Return the atoms comprising the omega dihedral.
/** If all atoms cannot be found, an empty list is returned.
    \see Residue */
IMPATOMEXPORT Atoms get_omega_dihedral_atoms(Residue rd);

//! Return a list of lists of atom types comprising the chi dihedrals
//! for residue type rd.
/** An empty list in a list is returned for residues with no chi dihedrals
    \see Residue */
IMPATOMEXPORT Vector<AtomTypes> get_chi_dihedral_atom_types(ResidueType rt);

//! Return a list of lists of atoms comprising the chi dihedrals
//! for residue rd.
/** An empty list in a list is returned for residues with no chi dihedrals
    \see Residue */
IMPATOMEXPORT Vector<Atoms> get_chi_dihedral_atoms(Residue rd);

//! Return a list of lists of ParticleIndexQuads comprising the chi dihedrals
//! for residue rd.
//! This datatype is exportable to Python.
IMPATOMEXPORT ParticleIndexQuads get_chi_dihedral_particle_indexes(Residue rd);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DIHEDRALS_H */
