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

namespace {
  CHARMMResidueTopologyBase &get_residue(
              std::auto_ptr<CHARMMIdealResidueTopology> &residue,
              std::auto_ptr<CHARMMPatch> &patch) {
    if (residue.get()) {
      return *residue;
    } else {
      return *patch;
    }
  }

  void parse_dele_line(std::string line, CHARMMPatch &patch) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results.size() < 3) return;

    // Only DELE ATOM supported for now
    if (split_results[1] == "ATOM") {
      patch.add_deleted_atom(split_results[2]);
    }
  }

  void parse_angle_line(std::string line, CHARMMResidueTopologyBase &residue) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 3) {
      if (split_results[i][0] == '!') return;  // comments start
      std::vector<std::string> atoms(&split_results[i], &split_results[i+3]);
      residue.add_angle(atoms);
    }
  }

  void parse_dihedral_line(std::string line,
                           CHARMMResidueTopologyBase &residue) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 4) {
      if (split_results[i][0] == '!') return;  // comments start
      std::vector<std::string> atoms(&split_results[i], &split_results[i+4]);
      residue.add_dihedral(atoms);
    }
  }

  void parse_improper_line(std::string line,
                           CHARMMResidueTopologyBase &residue) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 4) {
      if (split_results[i][0] == '!') return;  // comments start
      std::vector<std::string> atoms(&split_results[i], &split_results[i+4]);
      residue.add_improper(atoms);
    }
  }

  void parse_patch_line(std::string line, std::string &first,
                        std::string &last) {
    const std::string FIRST = "FIRS";
    const std::string LAST = "LAST";

    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    for (unsigned int i = 1; i < split_results.size(); i += 2) {
      if (split_results[i][0] == '!') return;  // comments start
      if (split_results[i].substr(0, FIRST.length()) == FIRST) {
        first = split_results[i + 1];
        if (first == "NONE") first = "";
      } else if (split_results[i].substr(0, LAST.length()) == LAST) {
        last = split_results[i + 1];
        if (last == "NONE") last = "";
      }
    }
  }
}

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
  const String DEFA_LINE = "DEFA";
  const String PATC_LINE = "PATC";
  const String RESI_LINE = "RESI";
  const String PRES_LINE = "PRES";
  const String ATOM_LINE = "ATOM";
  const String DELE_LINE = "DELE";
  const String BOND_LINE = "BOND";
  const String BOND_LINE2 = "DOUBLE";
  const String ANGLE_LINE = "ANGL";
  const String DIHEDRAL_LINE = "DIHE";
  const String IMPROPER_LINE = "IMPR";
  std::string first_patch = "", last_patch = "";
  std::auto_ptr<CHARMMIdealResidueTopology> residue;
  std::auto_ptr<CHARMMPatch> patch;

  ResidueType curr_res_type;
  while (!input_file.eof()) {
    String line;
    getline(input_file, line);
    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    // read residue line
    if(line.substr(0, RESI_LINE.length()) == RESI_LINE) {
      if (residue.get()) {
        add_residue_topology(*residue.release());
      } else if (patch.get()) {
        add_patch(*patch.release());
      }
      curr_res_type = parse_residue_line(line);
      residue.reset(new CHARMMIdealResidueTopology(curr_res_type.get_string()));
      residue->set_default_first_patch(first_patch);
      residue->set_default_last_patch(last_patch);

    // handle patch residues
    } else if (line.substr(0, PRES_LINE.length()) == PRES_LINE) {
      if (residue.get()) {
        add_residue_topology(*residue.release());
      } else if (patch.get()) {
        add_patch(*patch.release());
      }
      std::vector<String> split_results;
      boost::split(split_results, line, boost::is_any_of(" "),
                   boost::token_compress_on);
      if (split_results.size() < 3) {
        IMP_THROW("Invalid PRES line: " << line, ValueException);
      }
      patch.reset(new CHARMMPatch(split_results[1]));

    // handle DEFA line
    } else if (line.substr(0, DEFA_LINE.length()) == DEFA_LINE) {
      parse_patch_line(line, first_patch, last_patch);

    // handle PATC line
    } else if (line.substr(0, PATC_LINE.length()) == PATC_LINE
               && residue.get()) {
      std::string first = residue->get_default_first_patch();
      std::string last = residue->get_default_last_patch();
      parse_patch_line(line, first, last);
      residue->set_default_first_patch(first);
      residue->set_default_last_patch(last);

    // read DELE line
    } else if (line.substr(0, DELE_LINE.length()) == DELE_LINE
               && patch.get()) {
      parse_dele_line(line, *patch);

    // read atom line
    } else if (line.substr(0, ATOM_LINE.length()) == ATOM_LINE
               && (residue.get() || patch.get())) {
      parse_atom_line(line, curr_res_type, get_residue(residue, patch));

    // read bond line
    } else if ((line.substr(0, BOND_LINE.length()) == BOND_LINE ||
               line.substr(0, BOND_LINE2.length()) == BOND_LINE2)
               && (residue.get() || patch.get())) {
      parse_bond_line(line, curr_res_type, get_residue(residue, patch));

    // read angle line
    } else if (line.substr(0, ANGLE_LINE.length()) == ANGLE_LINE
               && (residue.get() || patch.get())) {
      parse_angle_line(line, get_residue(residue, patch));
    // read dihedral line
    } else if (line.substr(0, DIHEDRAL_LINE.length()) == DIHEDRAL_LINE
               && (residue.get() || patch.get())) {
      parse_dihedral_line(line, get_residue(residue, patch));
    // read improper line
    } else if (line.substr(0, IMPROPER_LINE.length()) == IMPROPER_LINE
               && (residue.get() || patch.get())) {
      parse_improper_line(line, get_residue(residue, patch));
    }
  }
  if (residue.get()) {
    add_residue_topology(*residue);
  } else if (patch.get()) {
    add_patch(*patch);
  }
}

ResidueType CharmmParameters::parse_residue_line(const String& line) {
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) {
    IMP_THROW("Invalid RESI line: " << line, ValueException);
  }
  String curr_residue = split_results[1];
  if (ResidueType::get_key_exists(curr_residue)) {
       return ResidueType(curr_residue);
   } else {
       // assume charmm is correct
       return ResidueType(ResidueType::add_key(curr_residue));
   }
}

void CharmmParameters::parse_atom_line(const String& line,
                                       ResidueType curr_res_type,
                                       CHARMMResidueTopologyBase &residue)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 4) return; // ATOM line has at least 4 fields

  CHARMMAtomTopology atom(split_results[1]);
  atom.set_charmm_type(split_results[2]);
  atom.set_charge(atof(split_results[3].c_str()));
  residue.add_atom(atom);

  AtomType imp_atom_type;
  if (AtomType::get_key_exists(atom.get_name())) {
     imp_atom_type = AtomType(atom.get_name());
  } else {
     // assume charm is correct and this is a ATOM record
     // and it will be parsed right for elements
     imp_atom_type= AtomType(AtomType::add_key(atom.get_name()));
  }
  // save in map
  if(atom_res_type_2_force_field_atom_type_.find(curr_res_type) ==
     atom_res_type_2_force_field_atom_type_.end()) {
    atom_res_type_2_force_field_atom_type_[curr_res_type] = AtomTypeMap();
  }
  atom_res_type_2_force_field_atom_type_[curr_res_type].insert(
           std::make_pair(imp_atom_type,
                          std::make_pair(atom.get_charmm_type(),
                                         atom.get_charge())));
}

void CharmmParameters::parse_bond_line(const String& line,
                                       ResidueType curr_res_type,
                                       CHARMMResidueTopologyBase &residue)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) return; // BOND line has at least 3 fields

  std::vector<Bond> bonds;
  for(unsigned int i=1; i<split_results.size(); i+=2) {
    if(split_results[i][0] == '!') return;  // comments start
    std::vector<std::string> atoms(&split_results[i], &split_results[i+2]);
    residue.add_bond(atoms);
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


void CharmmParameters::do_show(std::ostream &out) const {
}

CHARMMTopology *CharmmParameters::make_topology(Hierarchy hierarchy)
{
  IMP_NEW(CHARMMTopology, topology, ());

  HierarchiesTemp chains = get_by_type(hierarchy, CHAIN_TYPE);

  for (HierarchiesTemp::iterator chainit = chains.begin();
       chainit != chains.end(); ++chainit) {
    IMP_NEW(CHARMMSegmentTopology, segment, ());
    HierarchiesTemp residues = get_by_type(*chainit, RESIDUE_TYPE);
    for (HierarchiesTemp::iterator resit = residues.begin();
         resit != residues.end(); ++resit) {
      std::string restyp = Residue(*resit).get_residue_type().get_string();
      IMP_NEW(CHARMMResidueTopology, residue, (get_residue_topology(restyp)));
      segment->add_residue(residue);
    }
    topology->add_segment(segment);
  }

  return topology.release();
}

IMPATOM_END_NAMESPACE
