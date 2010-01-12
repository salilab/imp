/**
 *  \file atom/pdb.h
 *  \brief Functions to read pdbs
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

/** @name PDB Reading

   The read PDB methods produce a hierarchy that looks as follows:
    - One Atom per ATOM or HETATM record in the PDB.
    - All Atom particles have a parent which is a Residue.
    - All Residue particles have a parent which is a Chain.

    Waters are currently dropped if they are ATOM records. This can be fixed.

    The read_pdb() functions should successfully parse all valid pdb files. It
    can produce warnings on files which are not valid. It will attempt to read
    such files, but all bets are off.

    When reading PDBs, Selector objects can be used to choose to only process
    certain record types. See the class documentation for more information.

    Set the IMP::LogLevel to IMP::VERBOSE to see details of parse errors.
*/
//!@{

/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy read_pdb(std::istream &in,
                                 Model* model);

/** Read a all the molecules in the first model of the
    pdb file.

    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy read_pdb(std::string pdb_file_name,
                                 Model* model);

/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::istream &in,
         Model* model,
         const Selector& selector,
         bool select_first_model = true,
         bool ignore_alternatives = true);

/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::string pdb_file_name,
         Model* model,
         const Selector& selector,
         bool select_first_model = true,
         bool ignore_alternatives = true);



/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchies read_multimodel_pdb(std::istream &in, Model *model,
                   const Selector& selector,
                   bool ignore_alternatives=true);

/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchies read_multimodel_pdb(String pdb_file_name,
                   Model *model,
                   const Selector& selector,
                   bool ignore_alternatives=true);
/** @} */

/** @name PDB Writing

    The methods to write a PDBs expects a Hierarchy that looks as follows:
    - all leaves are Atom particles
    - all Atom particles have Residue particles as parents
    - all Residue particles have Chain particles as an ancestor.

    The functions produce files that are not valid PDB files,
    i.e. only ATOM/HETATM lines are printed for all Atom particles
    in the hierarchy. Complain if your favorite program can't read them and
    we might fix it.
*/
//!@{

/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::ostream &out);
/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::string file_name);
/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(const Hierarchies &mhd,
                             std::ostream &out);
/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(const Hierarchies &mhd,
                             std::string file_name);

/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_multimodel_pdb(
                        const Hierarchies& mhd, std::ostream &out);
/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_multimodel_pdb(
                        const Hierarchies& mhd, std::string file_name);

/** @} */

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

/**
   This function returns a connectivity string in PDB format
  /note The CONECT records specify connectivity between atoms for which
      coordinates are supplied. The connectivity is described using
      the atom serial number as found in the entry.
  /note http://www.bmsc.washington.edu/CrystaLinks/man/pdb/guide2.2_frame.html
*/
IMPATOMEXPORT std::string conect_record_string(int,int);


IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
