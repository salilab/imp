/**
 * \file CHARMMParameters \brief access to CHARMMParameters
 *  force field parameters
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/charmm_segment_topology.h>
#include <IMP/atom/angle_decorators.h>

#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>

IMPATOM_BEGIN_NAMESPACE

namespace {
  CHARMMResidueTopologyBase *get_residue(
              Pointer<CHARMMIdealResidueTopology> &residue,
              Pointer<CHARMMPatch> &patch) {
    if (residue) {
      return residue;
    } else {
      return patch;
    }
  }

  // Map CHARMM HSD residue type to PDB HIS
  std::string get_residue_name(std::string residue_name,
                               bool translate_names_to_pdb)
  {
    if (translate_names_to_pdb && residue_name == "HSD") {
      return "HIS";
    } else {
      return residue_name;
    }
  }

  std::string get_atom_name(std::string atom_name,
                            CHARMMResidueTopologyBase *residue,
                            bool translate_names_to_pdb)
  {
    if (!translate_names_to_pdb) {
      return atom_name;
    }

    typedef std::map<std::string, std::string> ResidueMap;
    static std::map<std::string, ResidueMap> map;
    static bool map_init = false;
    // Initialize map on first call
    if (!map_init) {
      map_init = true;
      map["ALA"] = boost::assign::map_list_of("HN", "H");
      map["ARG"] = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("HD1", "HD2")("HD2", "HD3")
                                             ("HG1", "HG2")("HG2", "HG3");
      map["ASP"] = map["ASN"]
                 = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3");
      map["CYS"] = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("HG1", "HG");
      map["GLN"] = map["GLU"]
                 = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("HG1", "HG2")("HG2", "HG3");
      map["GLY"] = boost::assign::map_list_of("HN", "H")
                                             ("HA1", "HA2")("HA2", "HA3");
      map["ILE"] = boost::assign::map_list_of("HN", "H")("CD", "CD1")
                                             ("HG11", "HG12")("HG12", "HG13")
                                             ("HD1", "HD11")("HD2", "HD12")
                                             ("HD3", "HD13");
      map["LEU"] = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("CD1", "CD2")("CD2", "CD1")
                                             ("HD11", "HD21")("HD12", "HD22")
                                             ("HD13", "HD23")
                                             ("HD21", "HD11")("HD22", "HD12")
                                             ("HD23", "HD13");
      map["LYS"] = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("HG1", "HG2")("HG2", "HG3")
                                             ("HD1", "HD2")("HD2", "HD3")
                                             ("HE1", "HE2")("HE2", "HE3");
      map["MET"] = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3")
                                             ("HG1", "HG2")("HG2", "HG3");
      map["PRO"] = boost::assign::map_list_of("HB1", "HB2")("HB2", "HB3")
                                             ("HG1", "HG2")("HG2", "HG3")
                                             ("HD1", "HD2")("HD2", "HD3");
      map["SER"] = boost::assign::map_list_of("HN", "H")("HG1", "HG")
                                             ("HB1", "HB2")("HB2", "HB3");
      map["THR"] = map["VAL"]
                 = boost::assign::map_list_of("HN", "H");
      map["TRP"] = map["TYR"] = map["HIS"] = map["PHE"]
                 = boost::assign::map_list_of("HN", "H")
                                             ("HB1", "HB2")("HB2", "HB3");
      map["TIP3"] = boost::assign::map_list_of("OH2", "O");
      map["CAL"] = boost::assign::map_list_of("CAL", "CA");
      map["ALA"] = boost::assign::map_list_of("HN", "H");
      map["NTER"] = boost::assign::map_list_of("HT1", "H")("HT2", "H2")
                                              ("HT3", "H3");
      map["CTER"] = boost::assign::map_list_of("OT1", "O")("OT2", "OXT");
      map["GLYP"] = boost::assign::map_list_of("HT1", "H")("HT2", "H2")
                                              ("HT3", "H3")("HA1", "HA2")
                                              ("HA2", "HA3");
      map["PROP"] = boost::assign::map_list_of("HN1", "H1")("HN2", "H2")
                                              ("HD1", "HD2")("HD2", "HD3");
      map["DISU"] = boost::assign::map_list_of("1HB1", "1HB2")("1HB2", "1HB3")
                                              ("2HB1", "2HB2")("2HB2", "2HB3")
                                              ("1HG1", "1HG")("2HG1", "2HG");
    }

    // Look up residue type
    std::map<std::string, ResidueMap>::iterator resit
                                            = map.find(residue->get_type());
    if (resit != map.end()) {
      // Look up atom name
      ResidueMap::iterator atit = resit->second.find(atom_name);
      if (atit != resit->second.end()) {
        return atit->second;
      }
    }
    return atom_name;
  }

  Strings get_atom_names(std::string *first, std::string *last,
                         CHARMMResidueTopologyBase *residue,
                         bool translate_names_to_pdb)
  {
    Strings ret;
    for (std::string *str = first; str != last; ++str) {
      ret.push_back(get_atom_name(*str, residue, translate_names_to_pdb));
    }
    return ret;
  }

  bool excess_patch_bond(const Strings &atom_names,
                         const CHARMMResidueTopologyBase *residue,
                         bool translate_names_to_pdb) {
    if (translate_names_to_pdb) {
      std::string restyp = residue->get_type();
      if (restyp == "NTER" || restyp == "GLYP") {
        return atom_names[0] == "H" || atom_names[1] == "H";
      } else if (restyp == "CTER") {
        return atom_names[0] == "O" || atom_names[1] == "O";
      }
    }
    return false;
  }

  bool excess_patch_removal(const std::string &atom_name,
                            const CHARMMPatch *patch,
                            bool translate_names_to_pdb) {
    if (translate_names_to_pdb) {
      std::string restyp = patch->get_type();
      if (restyp == "NTER" || restyp == "GLYP") {
        return atom_name == "HN";
      } else if (restyp == "CTER") {
        return atom_name == "O";
      }
    }
    return false;
  }

  void parse_dele_line(std::string line, CHARMMPatch *patch,
                       bool translate_names_to_pdb) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results.size() < 3) return;

    // Only DELE ATOM supported for now
    if (split_results[1] == "ATOM") {
      std::string atom_name = get_atom_name(split_results[2], patch,
                                            translate_names_to_pdb);
      if (!excess_patch_removal(atom_name, patch, translate_names_to_pdb)) {
        patch->add_removed_atom(atom_name);
      }
    }
  }

  void parse_angle_line(std::string line, CHARMMResidueTopologyBase *residue,
                        bool translate_names_to_pdb) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 3) {
      if (split_results[i][0] == '!') return;  // comments start
      residue->add_angle(get_atom_names(&split_results[i], &split_results[i+3],
                                        residue, translate_names_to_pdb));
    }
  }

  void parse_dihedral_line(std::string line,
                           CHARMMResidueTopologyBase *residue,
                           bool translate_names_to_pdb) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 4) {
      if (split_results[i][0] == '!') return;  // comments start
      residue->add_dihedral(get_atom_names(&split_results[i],
                                           &split_results[i+4],
                                           residue, translate_names_to_pdb));
    }
  }

  void parse_improper_line(std::string line,
                           CHARMMResidueTopologyBase *residue,
                           bool translate_names_to_pdb) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);

    for (unsigned int i = 1; i < split_results.size(); i += 4) {
      if (split_results[i][0] == '!') return;  // comments start
      residue->add_improper(get_atom_names(&split_results[i],
                                           &split_results[i+4],
                                           residue, translate_names_to_pdb));
    }
  }

  void parse_internal_coordinate_line(std::string line,
                                      CHARMMResidueTopologyBase *residue,
                                      bool translate_names_to_pdb) {
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results.size() < 10) return; // IC line has at least 10 fields

    Strings atom_names = get_atom_names(&split_results[1], &split_results[5],
                                        residue, translate_names_to_pdb);
    // Improper IC entries have a leading * on the third atom name
    bool improper = false;
    if (atom_names[2][0] == '*') {
      improper = true;
      atom_names[2] = atom_names[2].substr(1);
    }
    CHARMMInternalCoordinate ic(atom_names,
                                atof(split_results[5].c_str()),
                                atof(split_results[6].c_str()),
                                atof(split_results[7].c_str()),
                                atof(split_results[8].c_str()),
                                atof(split_results[9].c_str()), improper);
    residue->add_internal_coordinate(ic);
  }

  void parse_patch_line(std::string line, std::string &first,
                        std::string &last, bool translate_names_to_pdb) {
    const std::string FIRST = "FIRS";
    const std::string LAST = "LAST";

    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    for (unsigned int i = 1; i < split_results.size(); i += 2) {
      if (split_results[i][0] == '!') return;  // comments start
      if (split_results[i].substr(0, FIRST.length()) == FIRST) {
        first = split_results[i + 1];
        if (first == "NONE") {
          first = "";
        } else {
          first = get_residue_name(first, translate_names_to_pdb);
        }
      } else if (split_results[i].substr(0, LAST.length()) == LAST) {
        last = split_results[i + 1];
        if (last == "NONE") {
          last = "";
        } else {
          last = get_residue_name(last, translate_names_to_pdb);
        }
      }
    }
  }

  typedef std::map<Particle *, std::vector<IMP::atom::Bond> > BondMap;

  // Build a simple mapping from Particles to bonds that connect them. Note
  // that we cannot use the existing such mapping (the bond graph) in the
  // Bonded particles, since bonds may be a subset of all bonds.
  void make_bond_map(const Particles &bonds, BondMap &particle_bonds)
  {
    for (Particles::const_iterator it = bonds.begin(); it != bonds.end();
         ++it) {
      IMP::atom::Bond bd = IMP::atom::Bond(*it);
      Particle *p1 = bd.get_bonded(0).get_particle();
      Particle *p2 = bd.get_bonded(1).get_particle();
      particle_bonds[p1].push_back(bd);
      particle_bonds[p2].push_back(bd);
    }
  }

  Particle *get_other_end_of_bond(Particle *p, Bond bd)
  {
    Particle *p1 = bd.get_bonded(0).get_particle();
    Particle *p2 = bd.get_bonded(1).get_particle();
    return p1 == p ? p2 : p1;
  }

}

CHARMMParameters::CHARMMParameters(const String& top_file_name,
                                   const String& par_file_name,
                                   bool translate_names_to_pdb)
{
  // Parameter objects are not designed to be added into other containers
  set_was_used(true);

  std::ifstream top_file(top_file_name.c_str());
  if(!top_file) {
    IMP_THROW("Can't open topology file " << top_file_name,
              IOException);
  }
  read_topology_file(top_file, translate_names_to_pdb);
  top_file.close();

  if(!par_file_name.empty()) {
    std::ifstream par_file(par_file_name.c_str());
    if(!par_file) {
      IMP_THROW("Can't open charmm file " << par_file_name,
                  IOException);
    }
    read_parameter_file(par_file);
    par_file.close();
  }
}

void CHARMMParameters::read_topology_file(std::ifstream& input_file,
                                          bool translate_names_to_pdb)
{
  IMP_OBJECT_LOG;
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
  const String IC_LINE = "IC";
  std::string first_patch = "", last_patch = "";
  Pointer<CHARMMIdealResidueTopology> residue;
  Pointer<CHARMMPatch> patch;

  ResidueType curr_res_type;
  while (!input_file.eof()) {
    String line;
    getline(input_file, line);
    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    // read residue line
    if(line.substr(0, RESI_LINE.length()) == RESI_LINE) {
      if (residue) {
        add_residue_topology(residue.release());
      } else if (patch) {
        add_patch(patch.release());
      }
      curr_res_type = parse_residue_line(line, translate_names_to_pdb);
      ResidueType rt(curr_res_type.get_string());
      residue = new CHARMMIdealResidueTopology(rt);
      residue->set_default_first_patch(first_patch);
      residue->set_default_last_patch(last_patch);

    // handle patch residues
    } else if (line.substr(0, PRES_LINE.length()) == PRES_LINE) {
      if (residue) {
        add_residue_topology(residue.release());
      } else if (patch) {
        add_patch(patch.release());
      }
      std::vector<String> split_results;
      boost::split(split_results, line, boost::is_any_of(" "),
                   boost::token_compress_on);
      if (split_results.size() < 3) {
        IMP_THROW("Invalid PRES line: " << line, ValueException);
      }
      patch = new CHARMMPatch(get_residue_name(split_results[1],
                                               translate_names_to_pdb));

    // handle DEFA line
    } else if (line.substr(0, DEFA_LINE.length()) == DEFA_LINE) {
      parse_patch_line(line, first_patch, last_patch, translate_names_to_pdb);

    // handle PATC line
    } else if (line.substr(0, PATC_LINE.length()) == PATC_LINE
               && residue) {
      std::string first = residue->get_default_first_patch();
      std::string last = residue->get_default_last_patch();
      parse_patch_line(line, first, last, translate_names_to_pdb);
      residue->set_default_first_patch(first);
      residue->set_default_last_patch(last);

    // read DELE line
    } else if (line.substr(0, DELE_LINE.length()) == DELE_LINE
               && patch) {
      parse_dele_line(line, patch, translate_names_to_pdb);

    // read atom line
    } else if (line.substr(0, ATOM_LINE.length()) == ATOM_LINE
               && (residue || patch)) {
      parse_atom_line(line, curr_res_type, get_residue(residue, patch),
                      translate_names_to_pdb);

    // read bond line
    } else if ((line.substr(0, BOND_LINE.length()) == BOND_LINE ||
               line.substr(0, BOND_LINE2.length()) == BOND_LINE2)
               && (residue || patch)) {
      parse_bond_line(line, curr_res_type, get_residue(residue, patch),
                      translate_names_to_pdb);

    // read angle line
    } else if (line.substr(0, ANGLE_LINE.length()) == ANGLE_LINE
               && (residue || patch)) {
      parse_angle_line(line, get_residue(residue, patch),
                       translate_names_to_pdb);
    // read dihedral line
    } else if (line.substr(0, DIHEDRAL_LINE.length()) == DIHEDRAL_LINE
               && (residue || patch)) {
      parse_dihedral_line(line, get_residue(residue, patch),
                          translate_names_to_pdb);
    // read improper line
    } else if (line.substr(0, IMPROPER_LINE.length()) == IMPROPER_LINE
               && (residue || patch)) {
      parse_improper_line(line, get_residue(residue, patch),
                          translate_names_to_pdb);
    // read internal coordinate line
    } else if (line.substr(0, IC_LINE.length()) == IC_LINE
               && (residue || patch)) {
      parse_internal_coordinate_line(line, get_residue(residue, patch),
                                     translate_names_to_pdb);
    }
  }
  if (residue) {
    add_residue_topology(residue);
  } else if (patch) {
    add_patch(patch);
  }
}

ResidueType CHARMMParameters::parse_residue_line(const String& line,
                                                 bool translate_names_to_pdb)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) {
    IMP_THROW("Invalid RESI line: " << line, ValueException);
  }
  String curr_residue = get_residue_name(split_results[1],
                                         translate_names_to_pdb);
  if (ResidueType::get_key_exists(curr_residue)) {
       return ResidueType(curr_residue);
   } else {
       // assume charmm is correct
       return ResidueType(ResidueType::add_key(curr_residue));
   }
}

void CHARMMParameters::parse_atom_line(const String& line,
                                       ResidueType curr_res_type,
                                       CHARMMResidueTopologyBase *residue,
                                       bool translate_names_to_pdb)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 4) return; // ATOM line has at least 4 fields

  CHARMMAtomTopology atom(get_atom_name(split_results[1], residue,
                                        translate_names_to_pdb));
  atom.set_charmm_type(split_results[2]);
  atom.set_charge(atof(split_results[3].c_str()));
  residue->add_atom(atom);

  AtomType imp_atom_type;
  if (AtomType::get_key_exists(atom.get_name())) {
     imp_atom_type = AtomType(atom.get_name());
  } else {
     // assume charm is correct and this is a ATOM record
     // and it will be parsed right for elements
     // Note that we don't currently parse the MASS lines and so have
     // no way to determine the element for this atom
     imp_atom_type= add_atom_type(atom.get_name(), UNKNOWN_ELEMENT);
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

void CHARMMParameters::parse_bond_line(const String& line,
                                       ResidueType curr_res_type,
                                       CHARMMResidueTopologyBase *residue,
                                       bool translate_names_to_pdb)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if(split_results.size() < 3) return; // BOND line has at least 3 fields

  std::vector<Bond> bonds;
  for(unsigned int i=1; i<split_results.size(); i+=2) {
    if(split_results[i][0] == '!') return;  // comments start
    Strings atom_names = get_atom_names(&split_results[i], &split_results[i+2],
                                        residue, translate_names_to_pdb);
    if (excess_patch_bond(atom_names, residue, translate_names_to_pdb)) {
      continue;
    }
    residue->add_bond(atom_names);
    // + connects to the next residue
    if (atom_names[0][0] == '+' || atom_names[1][0] == '+') continue;
    // skip 2-residue patch bonds
    if (atom_names[0][0] == '1' || atom_names[0][0] == '2'
        || atom_names[1][0] == '1' || atom_names[1][0] == '2') {
      continue;
    }
    AtomType imp_atom_type1 = AtomType(atom_names[0]);
    AtomType imp_atom_type2 = AtomType(atom_names[1]);
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

void CHARMMParameters::parse_nonbonded_parameters_line(String line)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if (split_results.size() < 4)
    return; // non-bonded line has at least 4 fields

  String charmm_atom_type = split_results[0];
  float epsilon = atof(split_results[2].c_str());
  float radius = atof(split_results[3].c_str());
  force_field_2_vdW_[charmm_atom_type] = std::make_pair(epsilon, radius);
}

void CHARMMParameters::parse_bonds_parameters_line(String line)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if (split_results.size() < 4)
    return; // bonds line has at least 4 fields

  CHARMMBondParameters p;
  p.force_constant = atof(split_results[2].c_str());
  p.ideal = atof(split_results[3].c_str());
  bond_parameters_[internal::CHARMMBondNames(split_results[0],
                                             split_results[1])] = p;
}

void CHARMMParameters::parse_angles_parameters_line(String line)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if (split_results.size() < 5)
    return; // angles line has at least 5 fields

  CHARMMBondParameters p;
  p.force_constant = atof(split_results[3].c_str());
  p.ideal = atof(split_results[4].c_str());
  angle_parameters_[internal::CHARMMAngleNames(split_results[0],
                                               split_results[1],
                                               split_results[2])] = p;
}

void CHARMMParameters::parse_dihedrals_parameters_line(String line,
                                       DihedralParameters &param)
{
  std::vector<String> split_results;
  boost::split(split_results, line, boost::is_any_of(" "),
               boost::token_compress_on);
  if (split_results.size() < 7)
    return; // dihedrals line has at least 7 fields

  CHARMMDihedralParameters p;
  p.force_constant = atof(split_results[4].c_str());
  p.multiplicity = atoi(split_results[5].c_str());
  p.ideal = atof(split_results[6].c_str());
  param.push_back(std::make_pair(
                    internal::CHARMMDihedralNames(split_results[0],
                                                  split_results[1],
                                                  split_results[2],
                                                  split_results[3]), p));
}

void CHARMMParameters::read_parameter_file(std::ifstream& input_file) {
  IMP_OBJECT_LOG;
  const String BONDS_LINE = "BONDS";
  const String ANGLES_LINE = "ANGLES";
  const String DIHEDRALS_LINE = "DIHEDRALS";
  const String IMPROPER_LINE = "IMPROPER";
  const String NONBONDED_LINE = "NONBONDED";
  enum { NONE, BONDS, ANGLES, DIHEDRALS, IMPROPERS, NONBONDED } section = NONE;

  while (!input_file.eof()) {
    String line;
    getline(input_file, line);

    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    if (line.substr(0, NONBONDED_LINE.length()) == NONBONDED_LINE) {
      section = NONBONDED;
      getline(input_file, line); //remove second line of NONBONDED
    } else if (line.substr(0, BONDS_LINE.length()) == BONDS_LINE) {
      section = BONDS;
    } else if (line.substr(0, ANGLES_LINE.length()) == ANGLES_LINE) {
      section = ANGLES;
    } else if (line.substr(0, DIHEDRALS_LINE.length()) == DIHEDRALS_LINE) {
      section = DIHEDRALS;
    } else if (line.substr(0, IMPROPER_LINE.length()) == IMPROPER_LINE) {
      section = IMPROPERS;
    } else if (line.substr(0, 5) == "HBOND" || line.substr(0, 5) == "NBFIX") {
      section = NONE;
    } else if (line.substr(0, 3) == "END") {
      break;
    } else {
      switch(section) {
      case BONDS:
        parse_bonds_parameters_line(line);
        break;
      case ANGLES:
        parse_angles_parameters_line(line);
        break;
      case DIHEDRALS:
        parse_dihedrals_parameters_line(line, dihedral_parameters_);
        break;
      case IMPROPERS:
        parse_dihedrals_parameters_line(line, improper_parameters_);
        break;
      case NONBONDED:
        parse_nonbonded_parameters_line(line);
        break;
      default:
        break;
      }
    }
  }
  if(force_field_2_vdW_.size() == 0) {
    IMP_FAILURE("NONBONDED params not found in Charmm parameter file");
  }
}


void CHARMMParameters::do_show(std::ostream &) const {
}

String CHARMMParameters::get_force_field_atom_type(Atom atom) const
{
  IMP_OBJECT_LOG;
  // Override base class to use CHARMMAtom decorator
  static String empty_atom_type;
  if (CHARMMAtom::particle_is_instance(atom)) {
    return CHARMMAtom(atom).get_charmm_type();
  } else {
    IMP_WARN_ONCE("Atom " << atom << " does not have a known CHARMM type"
                  << std::endl, warn_context_);
    return empty_atom_type;
  }
}

CHARMMTopology *CHARMMParameters::create_topology(Hierarchy hierarchy) const
{
  IMP_OBJECT_LOG;
  IMP_NEW(CHARMMTopology, topology, (this));

  HierarchiesTemp chains = get_by_type(hierarchy, CHAIN_TYPE);

  for (HierarchiesTemp::iterator chainit = chains.begin();
       chainit != chains.end(); ++chainit) {
    IMP_NEW(CHARMMSegmentTopology, segment, ());
    HierarchiesTemp residues = get_by_type(*chainit, RESIDUE_TYPE);
    for (HierarchiesTemp::iterator resit = residues.begin();
         resit != residues.end(); ++resit) {
      std::string restyp = Residue(*resit).get_residue_type().get_string();
      try {
        IMP_NEW(CHARMMResidueTopology, residue, (get_residue_topology(restyp)));
        segment->add_residue(residue);
      } catch (ValueException) {
        // If residue type is unknown, add empty topology for this residue
        IMP_WARN_ONCE("Residue type " << restyp << " was not found in "
                      "topology library; using empty topology for this residue"
                      << std::endl, warn_context_);
        IMP_NEW(CHARMMResidueTopology, residue, (ResidueType(restyp)));
        segment->add_residue(residue);
      }
    }
    topology->add_segment(segment);
  }
  IMP_IF_LOG(VERBOSE) {
    warn_context_.dump_warnings();
  } else {
    warn_context_.clear_warnings();
  }
  // Topology objects are not designed to be added into other containers
  topology->set_was_used(true);
  return topology.release();
}

std::vector<std::pair<internal::CHARMMDihedralNames,
                      CHARMMDihedralParameters> >::const_iterator
CHARMMParameters::find_dihedral(DihedralParameters::const_iterator begin,
                                DihedralParameters::const_iterator end,
                                const internal::CHARMMDihedralNames &dihedral,
                                bool allow_wildcards) const
{
  IMP_OBJECT_LOG;
  DihedralParameters::const_iterator best = end;
  int best_match = internal::CHARMMDihedralNames::MISMATCH;
  for (DihedralParameters::const_iterator it = begin; it != end; ++it) {
    int match = it->first.match(dihedral, allow_wildcards);
    if (match < best_match) {
      best_match = match;
      best = it;
      if (match == 0) break;
    }
  }
  return best;
}

Particles CHARMMParameters::create_angles(Particles bonds) const
{
  IMP_OBJECT_LOG;
  Particles ps;
  BondMap particle_bonds;
  make_bond_map(bonds, particle_bonds);

  // Iterate over all bonds
  for (Particles::const_iterator bit1 = bonds.begin();
       bit1 != bonds.end(); ++bit1) {
    IMP::atom::Bond bd = IMP::atom::Bond(*bit1);
    Particle *p2 = bd.get_bonded(0).get_particle();
    Particle *p3 = bd.get_bonded(1).get_particle();

    // Extend along each adjoining p2 bond to get candidate p1-p2-p3 angles
    for (std::vector<IMP::atom::Bond>::const_iterator bit2
         = particle_bonds[p2].begin();
         bit2 != particle_bonds[p2].end(); ++bit2) {
      Particle *p1 = get_other_end_of_bond(p2, *bit2);
      // Avoid making angles where p1 == p3, and avoid double-counting
      if (p3 > p1) {
        add_angle(p1, p2, p3, ps);
      }
    }
    // Do the same for p2-p3-p4 angles
    for (std::vector<IMP::atom::Bond>::const_iterator bit2
         = particle_bonds[p3].begin();
         bit2 != particle_bonds[p3].end(); ++bit2) {
      Particle *p4 = get_other_end_of_bond(p3, *bit2);
      if (p4 < p2) {
        add_angle(p2, p3, p4, ps);
      }
    }
  }
  return ps;
}

void CHARMMParameters::add_angle(Particle *p1, Particle *p2, Particle *p3,
                                 Particles &ps) const
{
  IMP_OBJECT_LOG;
  Angle ad = Angle::setup_particle(new Particle(p1->get_model()),
                                   core::XYZ(p1), core::XYZ(p2),
                                   core::XYZ(p3));
  try {
    const CHARMMBondParameters &p
          = get_angle_parameters(CHARMMAtom(p1).get_charmm_type(),
                                 CHARMMAtom(p2).get_charmm_type(),
                                 CHARMMAtom(p3).get_charmm_type());
    ad.set_ideal(p.ideal / 180.0 * PI);
    ad.set_stiffness(std::sqrt(p.force_constant * 2.0));
  } catch (const IndexException &e) {
    // If no parameters, warn only
    IMP_WARN(e.what() << std::endl);
  }
  ps.push_back(ad);
}

Particles CHARMMParameters::create_dihedrals(Particles bonds) const
{
  IMP_OBJECT_LOG;
  Particles ps;
  BondMap particle_bonds;
  make_bond_map(bonds, particle_bonds);

  // Iterate over all bonds
  for (Particles::const_iterator bit1 = bonds.begin();
       bit1 != bonds.end(); ++bit1) {
    IMP::atom::Bond bd = IMP::atom::Bond(*bit1);
    Particle *p2 = bd.get_bonded(0).get_particle();
    Particle *p3 = bd.get_bonded(1).get_particle();

    // Extend along each bond from p2 and p3 to get candidate
    // p1-p2-p3-p4 dihedrals
    for (std::vector<IMP::atom::Bond>::const_iterator bit2
         = particle_bonds[p2].begin();
         bit2 != particle_bonds[p2].end(); ++bit2) {
      Particle *p1 = get_other_end_of_bond(p2, *bit2);

      if (p1 != p3) {
        for (std::vector<IMP::atom::Bond>::const_iterator bit3
             = particle_bonds[p3].begin();
             bit3 != particle_bonds[p3].end(); ++bit3) {
          Particle *p4 = get_other_end_of_bond(p3, *bit3);

          // Avoid generating dihedrals for three-membered rings
          if (p1 != p4 && p2 != p4) {
            add_dihedral(p1, p2, p3, p4, ps);
          }
        }
      }
    }
  }
  return ps;
}

void CHARMMParameters::add_dihedral(Particle *p1, Particle *p2, Particle *p3,
                                    Particle *p4, Particles &ps) const
{
  IMP_OBJECT_LOG;
  try {
    std::vector<CHARMMDihedralParameters> p
          = get_dihedral_parameters(CHARMMAtom(p1).get_charmm_type(),
                                    CHARMMAtom(p2).get_charmm_type(),
                                    CHARMMAtom(p3).get_charmm_type(),
                                    CHARMMAtom(p4).get_charmm_type());
    for (std::vector<CHARMMDihedralParameters>::const_iterator it = p.begin();
         it != p.end(); ++it) {
      Dihedral dd = Dihedral::setup_particle(new Particle(p1->get_model()),
                                             core::XYZ(p1), core::XYZ(p2),
                                             core::XYZ(p3), core::XYZ(p4));
      dd.set_ideal(it->ideal / 180.0 * PI);
      dd.set_multiplicity(it->multiplicity);
      if (it->force_constant < 0.0) {
        dd.set_stiffness(-std::sqrt(-it->force_constant * 2.0));
      } else {
        dd.set_stiffness(std::sqrt(it->force_constant * 2.0));
      }
      ps.push_back(dd);
    }
  } catch (const IndexException &e) {
    // If no parameters, warn, and create an empty dihedral
    IMP_WARN(e.what() << std::endl);
    Dihedral dd = Dihedral::setup_particle(new Particle(p1->get_model()),
                                           core::XYZ(p1), core::XYZ(p2),
                                           core::XYZ(p3), core::XYZ(p4));
    ps.push_back(dd);
  }
}


CHARMMParameters* get_heavy_atom_CHARMM_parameters() {
  static IMP::Pointer<CHARMMParameters> ret
    =new CHARMMParameters(get_data_path("top_heav.lib"),
                          get_data_path("par.lib"));
  ret->set_log_level(SILENT);
  return ret;
}

CHARMMParameters* get_all_atom_CHARMM_parameters() {
  static IMP::Pointer<CHARMMParameters> ret
    =new CHARMMParameters(get_data_path("top.lib"),
                          get_data_path("par.lib"));
  ret->set_log_level(SILENT);
  return ret;
}

IMPATOM_END_NAMESPACE
