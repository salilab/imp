/*
 * \file internal/mol2.h
 * \brief a class with static functions for parsing mol2 files
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_MOL_2_H
#define IMPATOM_INTERNAL_MOL_2_H

#include <IMP/atom/atom_config.h>
#include "../Atom.h"
#include <IMP/base/map.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>

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
                   const base::map<int, Particle*>& molecule_atoms);

//! in the mol2 file the atom type string starts at atom_type_field_
static const unsigned mol2_type_field_ = 47;


IMPATOMEXPORT bool check_arbond(Particle* atom_p);

IMPATOMEXPORT std::string get_mol2_name(Atom at);

enum Subtype {ST_NONE=0, ST_AR=1, ST_AM=2};

IMPATOMEXPORT std::pair<AtomType, Subtype>
get_atom_type_from_mol2(std::string name);

IMPATOMEXPORT IntKey get_subtype_key();

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_MOL_2_H */
