/*
 * \file internal/mol2.h
 * \brief a class with static functions for parsing mol2 files
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_MOL_2_H
#define IMPATOM_INTERNAL_MOL_2_H

#include "../config.h"
#include "../Atom.h"
#include "IMP/Particle.h"
#include "IMP/atom/Atom.h"
#include <IMP/base_types.h>
#include <map>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Return true if "mol2" file_type
IMPATOMEXPORT bool is_mol2file_rec(const String& file_name_type);

//! Return true if "@<TRIPOS>MOLECULE" record.
IMPATOMEXPORT bool is_MOLECULE_rec(const String& mol2_line);

//! Return true if "@<TRIPOS>ATOM" record.
IMPATOMEXPORT bool is_MOL2ATOM_rec(const String& mol2_line);

//! Return true if "@<TRIPOS>BOND" record.
IMPATOMEXPORT bool is_BOND_rec(const String& mol2_line);

//! Return the first word in the title line
IMPATOMEXPORT String get_molecule_name(const String& mol2_line);

//! Return the first word in the type line
IMPATOMEXPORT String get_molecule_type(const String& mol2_line);

//! Return the second part of an atom line
IMPATOMEXPORT String pick_mol2atom_type(const String& atom_line);

//! Return true if one atom in the bond pair is deleted
IMPATOMEXPORT bool is_ATOM_del(const String& bond_line,
                               const std::map<int, Particle*>& molecule_atoms);

//! in the mol2 file the atom type string starts at atom_type_field_
static const unsigned mol2_type_field_ = 47;


IMPATOMEXPORT bool check_arbond(Particle* atom_p);

IMPATOMEXPORT std::string get_mol2_name(Atom at);

IMPATOMEXPORT AtomType get_atom_type_from_mol2(std::string name);

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_MOL_2_H */
