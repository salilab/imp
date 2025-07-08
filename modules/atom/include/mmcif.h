/**
 *  \file IMP/atom/mmcif.h
 *  \brief Functions to read PDBs in mmCIF or BinaryCIF format
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
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
                         PDBSelector *selector = get_default_pdb_selector(),
                         bool select_first_model = true
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        );


//! Read all models from the BinaryCIF file.
IMPATOMEXPORT Hierarchies read_multimodel_bcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        );

//! Read all the molecules in the first model of the BinaryCIF file.
IMPATOMEXPORT Hierarchy read_bcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector(),
                         bool select_first_model = true
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        );

//! Read all the molecules in the first model of the PDB-like file.
/** This should handle reading coordinates from any of the file formats
    used by the PDB database, i.e. legacy "PDB" format, mmCIF, or BinaryCIF.
    If the filename ends in '.cif' it is treated as an mmCIF file;
    if the filename ends in '.bcif' it is treated as a BinaryCIF file;
    otherwise a legacy PDB file is read.
    Note that TextInputs created from streams don't have a name and so
    will always be treated as legacy PDB. */
inline Hierarchy read_pdb_any(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector(),
                         bool select_first_model = true
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        ) {
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_mmcif(input, model, selector, select_first_model, noradii);
  } else if (filename.find(".bcif") == filename.size() - 5) {
    return read_bcif(input, model, selector, select_first_model, noradii);
  } else {
    return read_pdb(input, model, selector, select_first_model, noradii);
  }
}

//! Read all models from the PDB-like file.
/** This should handle reading coordinates from any of the file formats
    used by the PDB database, i.e. legacy "PDB" format, mmCIF, or BinaryCIF.
    If the filename ends in '.cif' it is treated as an mmCIF file;
    if the filename ends in '.bcif' it is treated as a BinaryCIF file;
    otherwise a legacy PDB file is read.
    Note that TextInputs created from streams don't have a name and so
    will always be treated as legacy PDB. */
inline Hierarchies read_multimodel_pdb_any(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                         , bool noradii = false
#endif
                        ) {
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_multimodel_mmcif(input, model, selector, noradii);
  } else if (filename.find(".bcif") == filename.size() - 5) {
    return read_multimodel_bcif(input, model, selector, noradii);
  } else {
    return read_multimodel_pdb(input, model, selector, noradii);
  }
}

#ifndef IMP_DOXYGEN
IMPATOM_DEPRECATED_FUNCTION_DECL(2.23)
inline Hierarchy read_pdb_or_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector(),
                         bool select_first_model = true
                         , bool noradii = false
                        ) {
  IMPATOM_DEPRECATED_FUNCTION_DEF(2.23, "Use read_pdb_any() instead");
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_mmcif(input, model, selector, select_first_model, noradii);
  } else {
    return read_pdb(input, model, selector, select_first_model, noradii);
  }
}

IMPATOM_DEPRECATED_FUNCTION_DECL(2.23)
inline Hierarchies read_multimodel_pdb_or_mmcif(TextInput input, Model *model,
                         PDBSelector *selector = get_default_pdb_selector()
                         , bool noradii = false
                        ) {
  IMPATOM_DEPRECATED_FUNCTION_DEF(
           2.23, "Use read_multimodel_pdb_any() instead");
  std::string filename = input.get_name();
  if (filename.find(".cif") == filename.size() - 4) {
    return read_multimodel_mmcif(input, model, selector, noradii);
  } else {
    return read_multimodel_pdb(input, model, selector, noradii);
  }
}
#endif

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MMCIF_H */
