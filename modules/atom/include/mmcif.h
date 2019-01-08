/**
 *  \file IMP/atom/mmcif.h
 *  \brief Functions to read PDBs in mmCIF format
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MMCIF_H
#define IMPATOM_MMCIF_H

#include <IMP/atom/atom_config.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/pdb.h>

IMPATOM_BEGIN_NAMESPACE

//! Read all models from the mmCIF file.
IMPATOMEXPORT Hierarchies read_multimodel_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        );

//! Read all the molecules in the first model of the mmCIF file.
IMPATOMEXPORT Hierarchy read_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        );

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MMCIF_H */
