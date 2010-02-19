/**
 *  \file mol2 reader
 *  \brief A class with static functions for parsing mol2 file
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/internal/mol2.h>
#include <IMP/atom/bond_decorators.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

bool is_mol2file_rec(const String& file_name_type)
{
  return(file_name_type.find("mol2") != String::npos);
}

bool is_MOLECULE_rec(const String& mol2_line)
{
  return(mol2_line.find("@<TRIPOS>MOLECULE") != std::string::npos);
}

bool is_MOL2ATOM_rec(const String& mol2_line)
{
  return(mol2_line.find("@<TRIPOS>ATOM") != std::string::npos);
}

bool is_BOND_rec(const String& mol2_line)
{
  return(mol2_line.find("@<TRIPOS>BOND") != std::string::npos);
}

String get_molecule_name(const String& mol2_line)
{
  std::stringstream ins(mol2_line);
  String molecule_name;
  ins >> molecule_name;
  return molecule_name;
}

String get_molecule_type(const String& mol2_line)
{
  std::stringstream ins(mol2_line);
  String molecule_type;
  ins >> molecule_type;
  return molecule_type;
}

String pick_mol2atom_type(const String& atom_line)
{
  String line_part2 = atom_line.substr(mol2_type_field_);
  return line_part2;
}

bool is_ATOM_del(const String& bond_line,
                 const std::map<int, Particle*>& molecule_atoms)
{
  int bond_number, atom_a_id, atom_b_id;
  sscanf(bond_line.c_str(), "%i %i %i", &bond_number, &atom_a_id, &atom_b_id);
  if (molecule_atoms.find(atom_a_id) == molecule_atoms.end()
     || molecule_atoms.find(atom_b_id) == molecule_atoms.end())
    return true;
  else
    return false;
}


bool check_arbond(Particle* atom_p) {
  Int bond_number, type, i;
  Int count_ar=0;
  Bond bond_d;

  Bonded bonded_d = Bonded(atom_p);
  bond_number = bonded_d.get_number_of_bonds();
  for(i=0; i<bond_number; i++) {
    bond_d = bonded_d.get_bond(i);
    type = bond_d.get_type();
    if(type == Bond::AROMATIC) {
      count_ar++;
    }
  }
  if (count_ar > 1) {
    return true;
  }
  else {
    return false;
  }
}


IMPATOM_END_INTERNAL_NAMESPACE
