/**
 *  \file IMP/atom/secondary_structure_reader.h
 *  \brief Functions to read PSIPRED/DSSP and encode as SSES
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SECONDARY_STRUCTURE_READER_H
#define IMPATOM_SECONDARY_STRUCTURE_READER_H

#include "atom_config.h"
#include "Hierarchy.h"
#include "Atom.h"
#include "SecondaryStructureResidue.h"
#include <IMP/file.h>

IMPATOM_BEGIN_NAMESPACE

/** @name PSIPRED reading
    Reads in PSIPRED results, Creating particles based on the sequence and
    decorates them as SecondaryStructureResidues
*/
IMPATOMEXPORT SecondaryStructureResidues read_psipred(base::TextInput inf,
                                                      Model * mdl);
/** @name PSIPRED reading
    Reads in PSIPRED results and decorates particles as
    SecondaryStructureResidues. Currently assuming order of ps matches file.
*/
IMPATOMEXPORT SecondaryStructureResidues read_psipred(base::TextInput inf,
                                                      Particles ps);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SECONDARY_STRUCTURE_READER_H */
