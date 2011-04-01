/**
 *  \file dihedrals.h     \brief Helpers to extract dihedral information.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DIHEDRALS_H
#define IMPATOM_DIHEDRALS_H

#include "atom_config.h"
#include "Atom.h"
#include "Residue.h"

IMPATOM_BEGIN_NAMESPACE

/** Return the atoms comprising the phi dihedral.
    If all atoms cannot be found, an empty list is returned.
    \relatesalso Residue */
IMPATOMEXPORT Atoms get_phi_dihedral(Residue rd);

/** Return the atoms comprising the psi dihedral.
    If all atoms cannot be found, an empty list is returned.
    \relatesalso Residue */
IMPATOMEXPORT Atoms get_psi_dihedral(Residue rd);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_DIHEDRALS_H */
