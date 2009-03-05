/**
 *  \file internal/pdb.h
 *  \brief A class with static functions for parsing PDB files
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_INTERNAL_PDB_H
#define IMPATOM_INTERNAL_PDB_H

#include "../config.h"

#include <IMP/base_types.h>

#include <vector>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Returns true if the given record is an ATOM record.
IMPATOMEXPORT bool is_ATOM_rec(const String& pdb_line);

//! Returns true if the given record is a HETATM record.
IMPATOMEXPORT bool is_HETATM_rec(const String& pdb_line);

//! Returns true if the given record is a MODEL record.
IMPATOMEXPORT bool is_MODEL_rec(const String& pdb_line);

//! Returns true if the given record is a ENDMDL record.
IMPATOMEXPORT bool is_ENDMDL_rec(const String& pdb_line);

//! Returns true if the given record is a CONECT record.
IMPATOMEXPORT bool is_CONECT_rec(const String& pdb_line);

//! Returns an ATOM record's atom number.
IMPATOMEXPORT int atom_number(const String& pdb_line);

/** Returns a string with an atom type string.
    The atom type is a 4 charachter long field.
    the first character is space in many cases, but not always. */
IMPATOMEXPORT String atom_type(const String& pdb_line);

//! Returns an ATOM record's alternative location indicator.
IMPATOMEXPORT char atom_alt_loc_indicator(const String& pdb_line);

//! Returns a 3-character string with residue name
IMPATOMEXPORT String atom_residue_name(const String& pdb_line);

//! Returns an ATOM record's chain id.
IMPATOMEXPORT char  atom_chain_id(const String& pdb_line);

//! Returns an ATOM record's residue index.
IMPATOMEXPORT short atom_residue_number(const String& pdb_line);

/** code for insertion of residues. Returns the value stored in the
    atomResIndexCharField of the given PDB record (column 26).
    ATOM    279  CA  SER    60       8.961   9.167  37.481  1.00 20.10
    ATOM    285  CA  GLY    60A      5.640  10.901  38.407  1.00 21.41
    In this case for atom no. 285 the method will return 'A'
    and for the atom no. 279 it will return ' '
*/
IMPATOMEXPORT char atom_residue_icode(const String& pdb_line);

//! Returns an ATOM record's X coordinate.
IMPATOMEXPORT float atom_xcoord(const String& pdb_line);

//! Returns an ATOM record's Y coordinate.
IMPATOMEXPORT float atom_ycoord(const String& pdb_line);

//! Returns an ATOM record's Z coordinate.
IMPATOMEXPORT float atom_zcoord(const String& pdb_line);

/** Returns the occupancy, a measure of the fraction of molecules in the
    crystal in which the current atom actually occupies the specified position
*/
IMPATOMEXPORT float atom_occupancy(const String& pdb_line);

/** Returns the temperature factor, which is a measure of how much an atom
    oscillates or vibrates around the specified position
*/
IMPATOMEXPORT float atom_temp_factor(const String& pdb_line);

//! Returns element string (2 chars)
IMPATOMEXPORT String atom_element(const String& pdb_line);

//! Returns the connected atoms from the CONECT record
IMPATOMEXPORT std::vector<unsigned short>
connected_atoms(const String& pdb_line);

/** ATOM Record Format (from PDB)
    COLUMNS        DATA TYPE       CONTENTS
    ------------------------------------------------------------------------
    1 -  6         Record name     "ATOM  "
    7 - 11         Integer         Atom serial number.
    13 - 16        Atom            Atom name.
    17             Character       Alternate location indicator.
    18 - 20        Residue name    Residue name.
    22             Character       Chain identifier.
    23 - 26        Integer         Residue sequence number.
    27             AChar           Code for insertion of residues.
    31 - 38        Real(8.3)       Orthogonal coordinates for X in Angstroms.
    39 - 46        Real(8.3)       Orthogonal coordinates for Y in Angstroms.
    47 - 54        Real(8.3)       Orthogonal coordinates for Z in Angstroms.
    55 - 60        Real(6.2)       Occupancy.
    61 - 66        Real(6.2)       Temperature factor (Default = 0.0).
    73 - 76        LString(4)      Segment identifier, left-justified.
    77 - 78        LString(2)      Element symbol, right-justified.
    79 - 80        LString(2)      Charge on the atom.

    \note making them static is evil but I can't bother to fix them now.
*/
static const unsigned atom_entry_type_field_ = 0;
static const unsigned atom_number_field_ = 6;
static const unsigned atom_type_field_ = 12;
static const unsigned atom_alt_loc_field_ = 16;
static const unsigned atom_res_name_field_ = 17;
static const unsigned atom_chain_id_field_ = 21;
static const unsigned atom_res_number_field_ = 22;
static const unsigned atom_res_insertion_field_ = 26;
static const unsigned atom_xcoord_field_ = 30;
static const unsigned atom_ycoord_field_ = 38;
static const unsigned atom_zcoord_field_ = 46;
static const unsigned atom_occupancy_field_ = 54;
static const unsigned atom_temp_factor_field_ = 60;
static const unsigned atom_element_field_ = 76;

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_PDB_H */
