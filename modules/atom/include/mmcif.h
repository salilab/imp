/**
 *  \file IMP/atom/mmcif.h
 *  \brief Functions to read PDBs in mmCIF format
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MMCIF_H
#define IMPATOM_MMCIF_H

#include <IMP/atom/atom_config.h>
#include <IMP/atom/Hierarchy.h>

IMPATOM_BEGIN_NAMESPACE

//! Read all the molecules in the first model of the mmCIF file.
IMPATOMEXPORT Hierarchy read_mmcif(TextInput input, Model *model);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MMCIF_H */
