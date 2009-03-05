/**
 *  \file pdb.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include "config.h"
#include "internal/pdb.h"
#include "selectors.h"

#include <IMP/Model.h>
#include <IMP/Particle.h>

#include "MolecularHierarchyDecorator.h"

IMPATOM_BEGIN_NAMESPACE

/** reads PDB file into a model.
    Selectors can be used to define which atoms to read.
    Several basic selectors are implemented in PDBParser */
IMPATOMEXPORT MolecularHierarchyDecorator
read_pdb(String pdb_file_name,
         Model* model,
         const Selector& selector = Selector(),
         bool select_first_model = true,
         bool ignore_alternatives = true);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
