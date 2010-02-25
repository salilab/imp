/**
 *  \file atom/mol2.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_MOL_2_H
#define IMPATOM_MOL_2_H

#include "config.h"
#include "internal/mol2.h"
#include "Hierarchy.h"

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/file.h>

IMPATOM_BEGIN_NAMESPACE

//! A base class for choosing which Mol2 atoms to read
/** The Mol2Selector classes are designed to be used as
    temporaries and so should never be stored.
 */
class IMPATOMEXPORT Mol2Selector {
 public:
  virtual bool operator()(const std::string& atom_line) const=0;
  virtual ~Mol2Selector();
};

//! Read all atoms
class AllMol2Selector: public Mol2Selector {
  bool operator()(const std::string&) const {return true;}
};


//! Defines a selector that will pick only non-hydrogen atoms
class IMPATOMEXPORT NonhydrogenMol2Selector : public Mol2Selector {
 public:
  bool operator() (const std::string& atom_line) const;
};

/** @name Mol2 IO

    \imp can also read and write Mol2 files. As with read_pdb(), selector
    objects are used to determine which atoms are read.

    The read function produces a hierarchy containing the molecule. The write
    hierarchy writes all the Residue types in the hierarchy to the file.
    @{
*/
IMPATOMEXPORT Hierarchy read_mol2(TextInput mol2_file,
                                  Model* model,
                                  const Mol2Selector& mol2sel
                                  = AllMol2Selector());

IMPATOMEXPORT void write_mol2(Hierarchy rhd,
                              TextOutput file_name);

/** @} */

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MOL_2_H */
