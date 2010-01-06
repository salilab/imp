/**
 * \file CharmmParameters \brief access to CharmmParameters
 *  force field parameters
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/CharmmParameters.h>

#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_NAMESPACE

CharmmParameters::CharmmParameters(const String& top_file_name,
                                   const String& par_file_name)
{
  std::ifstream top_file(top_file_name.c_str());
  if(!top_file) {
    IMP_THROW("Can't open topology file " << top_file_name,
              IOException);
  }
  read_topology_file(top_file);
  top_file.close();

  if(!par_file_name.empty()) {
    std::ifstream par_file(par_file_name.c_str());
    if(!par_file) {
      IMP_THROW("Can't open charmm file " << par_file_name,
                  IOException);
    }
    read_VdW_params(par_file);
    par_file.close();
  }
}

void CharmmParameters::read_topology_file(std::ifstream& input_file) {
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

ResidueType CharmmParameters::parse_residue_line(const String& line) {
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) return UNK; // RESI line has at least 3 fields
  String curr_residue = split_results[1];
  if (ResidueType::get_key_exists(curr_residue)) {
       return ResidueType(curr_residue);
   } else {
       // assume charmm is correct
       return ResidueType(ResidueType::add_key(curr_residue));
   }
}

void CharmmParameters::parse_atom_line(const String& line,
                                       ResidueType curr_res_type)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 4) return; // ATOM line has at least 4 fields
  String atom_name = split_results[1];
  String charmm_atom_type = split_results[2];
  float charge = atof(split_results[3].c_str());
  AtomType imp_atom_type;
  if (AtomType::get_key_exists(atom_name)) {
     imp_atom_type = AtomType(atom_name);
  } else {
     // assume charm is correct and this is a ATOM record
     // and it will be parsed right for elements
     imp_atom_type= AtomType(AtomType::add_key(atom_name));
  }
  // save in map
  if(atom_res_type_2_force_field_atom_type_.find(curr_res_type) ==
     atom_res_type_2_force_field_atom_type_.end()) {
    atom_res_type_2_force_field_atom_type_[curr_res_type] = AtomTypeMap();
  }
  atom_res_type_2_force_field_atom_type_[curr_res_type].insert(
           std::make_pair(imp_atom_type,
                          std::make_pair(charmm_atom_type, charge)));
}

void CharmmParameters::parse_bond_line(const String& line,
                                       ResidueType curr_res_type)
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
    // skip funny added modeller records
    if (split_results[i].find(':') != std::string::npos
        || split_results[i+1].find(':') != std::string::npos) continue;
    AtomType imp_atom_type1 = AtomType(split_results[i]);
    AtomType imp_atom_type2 = AtomType(split_results[i+1]);
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

void CharmmParameters::read_VdW_params(std::ifstream& input_file) {
  const String NONBONDED_LINE = "NONBONDED";

  bool in_nonbonded = false;
  while (!input_file.eof()) {
    String line;
    getline(input_file, line);

    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    if(line.substr(0, NONBONDED_LINE.length()) == NONBONDED_LINE) {
      in_nonbonded=true;
      getline(input_file, line); //remove second line of NONBONDED
      continue;
    }

    if(line.substr(0, 5) == "HBOND" || line.substr(0, 3) == "END" ||
       line.substr(0, 5) == "NBFIX") break; // eof NONBONDED

    if(in_nonbonded) {
      std::vector<String> split_results;
      boost::split(split_results, line, boost::is_any_of(" "),
                   boost::token_compress_on);
      if(split_results.size() < 4)
        continue; // non-bonded line has at least 4 fields

      String charmm_atom_type = split_results[0];
      float epsilon = atof(split_results[2].c_str());
      float radius = atof(split_results[3].c_str());
      force_field_2_vdW_[charmm_atom_type] = std::make_pair(epsilon, radius);
    }
  }

  if(force_field_2_vdW_.size() == 0) {
    IMP_FAILURE("NONBONDED params not found in Charmm parameter file");
  }
}


void CharmmParameters::show(std::ostream &out) const {
  out << "Charmm parameters\n";
}

IMPATOM_END_NAMESPACE
