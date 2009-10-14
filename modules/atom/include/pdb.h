/**
 *  \file atom/pdb.h
 *  \brief Functions to read pdbs
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include "config.h"
#include "selectors.h"
#include "Hierarchy.h"
#include "Atom.h"
#include "element.h"

#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_NAMESPACE

/** @name PDB IO

    Reading/writing PDBs produces/expects a Hierarchy that looks as follows:
    - One Atom per ATOM or HETATM record in the PDB.
    - All Atom particles have a parent which is a Residue.
    - All Residue particles have a parent which is a Chain.

    Waters are currently dropped if they are ATOM records. This can be fixed.

    The read_pdb() functions should successfully parse all valid pdb files. It
    can produce warnings on files which are not valid. It will attempt to read
    such files, but all bets are off.

    Set the IMP::LogLevel to IMP::VERBOSE to see details of parse errors.
*/
//!@{

/** Selector objects can be used to define which atoms to read.
    \see write_pdb
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::istream &in,
         Model* model,
         const Selector& selector = Selector(),
         bool select_first_model = true,
         bool ignore_alternatives = true);

/** Selector objects can be used to define which atoms to read.
    \see write_pdb
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::string pdb_file_name,
         Model* model,
         const Selector& selector = Selector(),
         bool select_first_model = true,
         bool ignore_alternatives = true);

/** \note This function produces files that are not valid PDB files,
    i.e. only ATOM/HETATM lines are printed for all Atom particles
    in the hierarchy. Complain if your favorite program can't read them and
    we might fix it.
   \see read_pdb
   \see write_pdb(const Hierarchys& mhd,std::string file_name)
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::ostream &out);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::string file_name);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Hierarchies &mhd,
                             std::ostream &out);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Hierarchies &mhd,
                             std::string file_name);

/**
   This function returns a string in PDB ATOM format
*/
IMPATOMEXPORT std::string pdb_string(const algebra::Vector3D& v,
                                     int index = -1,
                                     const AtomType& at = AT_C,
                                     const ResidueType& rt = atom::ALA,
                                     char chain = ' ',
                                     int res_index = 1,
                                     char res_icode = ' ',
                                     Element e = C);

//!@}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
