/**
 * \file Topology \brief topology definitions
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/internal/Topology.h>

#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

Topology::Topology(const String& top_file_name) {
  std::ifstream top_file(top_file_name.c_str());
  if(!top_file) {
    std::cerr << "Can't open topology file " << top_file_name << std::endl;
    exit(1);
  }
  read_topology_file(top_file);
  top_file.close();
}

void Topology::add_bonds(Hierarchy mhd) {
  add_bonds(mhd, Hierarchy::RESIDUE);
  add_bonds(mhd, Hierarchy::NUCLEICACID);
  add_bonds(mhd, Hierarchy::FRAGMENT);
}

void Topology::add_bonds(Hierarchy mhd,
                         Hierarchy::Type type) {
  // get all residues
  Particles ps = get_by_type(mhd, type);
  Residue prev_rd;
  for(unsigned int i=0; i<ps.size(); i++) {
    Residue rd = Residue::cast(ps[i]);
    // add bonds to the current residue
    add_bonds(rd);
    // add bond between the residues (if same chain)
    if(i>0 && get_chain(prev_rd) == get_chain(rd) &&
       prev_rd.get_index() <= rd.get_index())
      add_bonds(prev_rd, rd);
    prev_rd = rd;
  }
}

void Topology::add_bonds(Residue rd1, Residue rd2) {
  Atom ad1, ad2;
  // connect two residues by C-N bond
  if(rd1.get_type() == Hierarchy::RESIDUE &&
     rd2.get_type() == Hierarchy::RESIDUE) {
    ad1 = get_atom(rd1, atom::AT_C);
    ad2 = get_atom(rd2, atom::AT_N);
   }
  // connect two nucleic acids by O3'-P bond
  if(rd1.get_type() == Hierarchy::NUCLEICACID &&
     rd2.get_type() == Hierarchy::NUCLEICACID) {
    ad1 = get_atom(rd1, atom::AT_O3p);
    ad2 = get_atom(rd2, atom::AT_P);
  }
  if(!ad1 || !ad2) return;
  Particle* p1 = ad1.get_particle();
  Particle* p2 = ad2.get_particle();

  Bonded b1,b2;
  if(Bonded::is_instance_of(p1)) b1 = Bonded::cast(p1);
  else b1 = Bonded::create(p1);

  if(Bonded::is_instance_of(p2)) b2 = Bonded::cast(p2);
  else b2 = Bonded::create(p2);

  IMP::atom::Bond bd = bond(b1, b2, IMP::atom::Bond::COVALENT);
}

void Topology::add_bonds(Residue rd) {
  ResidueType type = rd.get_residue_type();
  if(residue_bonds_.find(type) == residue_bonds_.end()) return;

  std::vector<Bond>& bonds = residue_bonds_[type];
  for(unsigned int i=0; i<bonds.size(); i++) {
    Atom ad1 = get_atom(rd, bonds[i].type1_);
    Atom ad2 = get_atom(rd, bonds[i].type2_);
    if(!ad1 || !ad2) continue;

    Particle* p1 = ad1.get_particle();
    Particle* p2 = ad2.get_particle();

    Bonded b1,b2;
    if(Bonded::is_instance_of(p1)) b1 = Bonded::cast(p1);
    else b1 = Bonded::create(p1);

    if(Bonded::is_instance_of(p2)) b2 = Bonded::cast(p2);
    else b2 = Bonded::create(p2);

    IMP::atom::Bond bd = bond(b1, b2, bonds[i].bond_type_);
  }
}

void Topology::read_topology_file(std::ifstream& input_file) {
  const String RESI_LINE = "RESI";
  const String PRES_LINE = "PRES"; //protonated
  const String ATOM_LINE = "ATOM";
  const String BOND_LINE = "BOND";
  const String BOND_LINE2 = "DOUBLE";

  bool in_residue = false;
  ResidueType curr_res_type;
  while (!input_file.eof()) {
    String line;
    getline(input_file, line);
    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    // read residue line
    if(line.substr(0, RESI_LINE.length()) == RESI_LINE ||
       line.substr(0, PRES_LINE.length()) == PRES_LINE) {
      in_residue = true;
      curr_res_type = parse_residue_line(line);
      continue;
    }

    // read atom line
    if(line.substr(0, ATOM_LINE.length()) == ATOM_LINE && in_residue) {
      parse_atom_line(line, curr_res_type);
      continue;
    }

    // read bond line
    if((line.substr(0, BOND_LINE.length()) == BOND_LINE ||
        line.substr(0, BOND_LINE2.length()) == BOND_LINE2)
       && in_residue) {
      parse_bond_line(line, curr_res_type);
    }
  }
}

ResidueType Topology::parse_residue_line(const String& line) {
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) return UNK; // RESI line has at least 3 fields
  String curr_residue = split_results[1];
  return ResidueType(curr_residue.c_str());
}

void Topology::parse_atom_line(const String& line,
                               const ResidueType& curr_res_type)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 4) return; // ATOM line has at least 4 fields
  String atom_name = split_results[1];
  String charmm_atom_type = split_results[2];
  float charge = atof(split_results[3].c_str());
  AtomType imp_atom_type = AtomType(atom_name.c_str());

  // save in map
  if(atom_res_type_2_charmm_atom_type_.find(curr_res_type) ==
     atom_res_type_2_charmm_atom_type_.end()) {
    atom_res_type_2_charmm_atom_type_[curr_res_type] = AtomTypeMap();
  }
  atom_res_type_2_charmm_atom_type_[curr_res_type].insert(
           std::make_pair(imp_atom_type,
                          std::make_pair(charmm_atom_type, charge)));
}

void Topology::parse_bond_line(const String& line,
                               const ResidueType& curr_res_type)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) return; // BOND line has at least 3 fields

  std::vector<Bond> bonds;
  for(unsigned int i=1; i<split_results.size(); i+=2) {
    if(split_results[i][0] == '!') return;  // comments start
    // + connects to the next residue
    if(split_results[i][0] == '+' || split_results[i+1][0] == '+') continue;
    AtomType imp_atom_type1 = AtomType(split_results[i].c_str());
    AtomType imp_atom_type2 = AtomType(split_results[i+1].c_str());
    Bond bond(imp_atom_type1, imp_atom_type2);
    bonds.push_back(bond);
  }

  if(residue_bonds_.find(curr_res_type) == residue_bonds_.end()) {
    residue_bonds_[curr_res_type] = bonds;
  } else {
    residue_bonds_[curr_res_type].insert(residue_bonds_[curr_res_type].end(),
                                         bonds.begin(), bonds.end());
  }
}

String Topology::get_charmm_atom_type(const AtomType& atom_type,
                                      const ResidueType& residue_type) const {
  static String empty_atom_type;
  if(atom_res_type_2_charmm_atom_type_.find(residue_type) ==
     atom_res_type_2_charmm_atom_type_.end()) {
    std::cerr << "Warning! Residue not found " << residue_type << std::endl;
    return empty_atom_type;
  }

  //  string fixedAtomName = fixAtomName(atomName, residueName, NORM);
  const AtomTypeMap& atom_map =
    atom_res_type_2_charmm_atom_type_.find(residue_type)->second;
  if(atom_map.find(atom_type) == atom_map.end()) {
    std::cerr << "Warning! Atom not found " << atom_type
              << " residue " << residue_type << std::endl;
    return empty_atom_type;
  }

  return atom_map.find(atom_type)->second.first;
}


IMPATOM_END_INTERNAL_NAMESPACE
