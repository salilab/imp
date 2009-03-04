/**
 *  \file PDBParser.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_PDB_READER_H
#define IMPATOM_PDB_READER_H

#include "PDBParser.h"
#include "config.h"
#include "selectors.h"

#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <IMP/core/MolecularHierarchyDecorator.h>

IMPATOM_BEGIN_NAMESPACE

class IMPATOMEXPORT PDBReader {
public:

  /** reads PDB file into a model.
   Selectors can be used to define which atoms to read.
  Several basic selectors are implemented in PDBParser */
  static core::MolecularHierarchyDecorator read_PDB_file(
               String pdb_file_name,
               Model* model,
               const Selector& selector = Selector(),
               bool select_first_model = true,
               bool ignore_alternatives = true);

  /** writes a Model into PDB file */
  static void write_PDB_file(String pdb_file_name, Model *model);

private:

  static Particle* root_particle(Model *m, const String& pdb_line);
  static Particle* chain_particle(Model *m, char chain_id);
  static Particle* residue_particle(Model *m, const String& pdb_line);
  static Particle* atom_particle(Model *m, const String& pdb_line);

  static void set_chain_type(const core::MolecularHierarchyDecorator& hrd,
                             core::MolecularHierarchyDecorator& hcd);
private:
  PDBReader() {};
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_READER_H */
