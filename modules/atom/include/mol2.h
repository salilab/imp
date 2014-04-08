/**
 *  \file IMP/atom/mol2.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MOL_2_H
#define IMPATOM_MOL_2_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include "atom_macros.h"
#include "internal/mol2.h"

#include <IMP/kernel/Model.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base/file.h>

IMPATOM_BEGIN_NAMESPACE

//! A base class for choosing which Mol2 atoms to read
/**
 */
class IMPATOMEXPORT Mol2Selector : public IMP::base::Object {
 public:
  Mol2Selector() : Object("Mol2Selector%1%") {}
  virtual bool get_is_selected(const std::string& atom_line) const = 0;
  virtual ~Mol2Selector();
};

//! Read all atoms
class AllMol2Selector : public Mol2Selector {
 public:
  IMP_MOL2_SELECTOR(AllMol2Selector, return (true || mol2_line.empty()),
                    out << "");
};

//! Defines a selector that will pick only non-hydrogen atoms
class IMPATOMEXPORT NonHydrogenMol2Selector : public Mol2Selector {
 public:
  IMP_MOL2_SELECTOR(NonHydrogenMol2Selector,
                    String atom_type = internal::pick_mol2atom_type(mol2_line);
                    return (atom_type[0] != 'H'), out << "");
};

/** @name Mol2 IO

    \imp can also read and write Mol2 files. As with read_pdb(), selector
    objects are used to determine which atoms are read.

    The read function produces a hierarchy containing the molecule. The write
    hierarchy writes all the Residue types in the hierarchy to the file.
    @{
*/
//! Create a hierarchy from a Mol2 file.
IMPATOMEXPORT Hierarchy read_mol2(base::TextInput mol2_file,
                                  kernel::Model* model,
                                  Mol2Selector* mol2sel = nullptr);

//! Write a lignand hierarchy as a mol2 file
/** For now, this has to be a hierarchy created by read_mol2()
 */
IMPATOMEXPORT void write_mol2(Hierarchy rhd, base::TextOutput file_name);

/** @} */

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MOL_2_H */
