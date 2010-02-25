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
  if (!Bonded::particle_is_instance(atom_p)) return false;
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

IMPATOMEXPORT std::string get_mol2_name(Atom at) {
  bool isar= check_arbond(at);
  std::string atom_type = at.get_atom_type().get_string();
  if (atom_type.find("HET:") == 0) {
    atom_type= std::string(atom_type, 4);
  }
  boost::trim(atom_type);
  if (isar) {
    atom_type += ".ar";
  }
  for (unsigned int i=0; i< atom_type.size(); ++i) {
    if (std::isdigit(atom_type[i], std::locale())) {
      atom_type.insert(i, ".");
      break;
    }
  }
  //std::cout << "Returning " << atom_type << " for " << at << std::endl;
  return atom_type;
}

IMPATOMEXPORT AtomType get_atom_type_from_mol2(std::string name) {
  boost::trim(name);
  std::string element(name, 0, name.find('.'));
  Element e= get_element_table().get_element(element);
  if (e== UNKNOWN_ELEMENT) {
    IMP_THROW("Can't deal with element " << name,
              IOException);
  }
  if (name.find(".ar") != std::string::npos) {
    name= std::string(name, 0, name.find('.'));
  }
  if (name.find('.') != std::string::npos) {
    name.erase(name.find('.'), 1);
  }
  std::string atom_name;
  if (name.size() ==1) {
    atom_name= std::string("HET: ") + name + "  ";
  } else if (name.size() ==2) {
    if (std::isdigit(name[1], std::locale())) {
      atom_name=std::string("HET: ") + name +" ";
    } else {
      atom_name=std::string("HET:") + name +"  ";
    }
  } else {
    IMP_THROW("Can't deal with atom name " << name,
              IOException);
  }
  if (!get_atom_type_exists(atom_name)) {
    add_atom_type(atom_name, e);
  }
  return AtomType(atom_name);
}


IMPATOM_END_INTERNAL_NAMESPACE
