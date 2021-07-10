/**
 *  \file IMP/atom/mmcif.h
 *  \brief Functions to read PDBs in mmCIF format
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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

//! Read all the molecules in the first model of the PDB or mmCIF file.
/** If the filename ends in '.cif' it is treated as an mmCIF file,
    otherwise a PDB file is read.
    Note that TextInputs created from streams don't have a name and so
    will always be treated as PDB. */
inline Hierarchy read_pdb_or_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        ) {
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_mmcif(input, model, selector, noradii);
  } else {
    return read_pdb(input, model, selector, true, noradii);
  }
}

//! Read all models from the PDB or mmCIF file.
/** If the filename ends in '.cif' it is treated as an mmCIF file,
    otherwise a PDB file is read.
    Note that TextInputs created from streams don't have a name and so
    will always be treated as PDB. */
inline Hierarchies read_multimodel_pdb_or_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        ) {
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_multimodel_mmcif(input, model, selector, noradii);
  } else {
    return read_multimodel_pdb(input, model, selector, noradii);
  }
}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MMCIF_H */
