/**
 *  \file PDBParser.h
 *  \brief A class with static functions for parsing PDB files
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/internal/pdb.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Molecule.h>
#include <IMP/core/provenance.h>
#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

Particle* atom_particle(Model* m, const std::string& atom_type,
                        Element element, bool is_hetatm,
                        int atom_index, int residue_index,
                        double x, double y, double z,
                        double occupancy, double temp_factor) {
  AtomType atom_name;
  std::string string_name = atom_type;
  // determine AtomType
  if (is_hetatm) {
    string_name = "HET:" + string_name;
    if (!get_atom_type_exists(string_name)) {
      atom_name = add_atom_type(string_name, element);
    } else {
      atom_name = AtomType(string_name);
    }
  } else {  // ATOM line
    boost::trim(string_name);
    if (string_name.empty()) {
      string_name = "UNK";
    }
    if (!AtomType::get_key_exists(string_name)) {
      IMP_LOG_VERBOSE("ATOM record type not found: \""
                      << string_name << "\" in PDB file " << std::endl);
      atom_name = add_atom_type(string_name, element);
    } else {
      atom_name = AtomType(string_name);
    }
  }
  // new particle
  Particle* p = new Particle(m);
  p->add_attribute(get_pdb_index_key(), atom_index);
  algebra::Vector3D v(x, y, z);
  // atom decorator
  Atom d = Atom::setup_particle(p, atom_name);
  std::ostringstream oss;
  oss << "Atom " + atom_name.get_string() << " of residue " << residue_index;
  p->set_name(oss.str());
  core::XYZ::setup_particle(p, v).set_coordinates_are_optimized(true);
  d.set_input_index(atom_index);
  d.set_occupancy(occupancy);
  d.set_temperature_factor(temp_factor);
  d.set_element(element);
  // check if the element matches
  Element e2 = get_element_for_atom_type(atom_name);
  if (element != e2) {
    IMP_LOG_VERBOSE(
        "AtomType element and PDB line elements don't match. AtomType "
        << e2 << " vs. determined from PDB " << element << std::endl);
  }
  return p;
}

IntKey get_pdb_index_key() {
  static IntKey pdb_index_key("pdb index");
  return pdb_index_key;
}

Particle *residue_particle(Model *m, int residue_index, char ins_code,
                           const std::string &residue_type) {
  Particle* p = new Particle(m);

  std::string rn = residue_type;
  boost::trim(rn);
  if (rn.empty()) {
    rn = "UNK";
  }
  ResidueType residue_name = ResidueType(rn);

  // residue decorator
  Residue::setup_particle(p, residue_name, residue_index, (int)ins_code);
  p->set_name(residue_name.get_string());
  return p;
}

Particle* chain_particle(Model* m, std::string chain_id, std::string filename) {
  Particle* p = new Particle(m);
  Chain::setup_particle(p, chain_id);
  p->set_name(std::string("Chain " + chain_id));
  Molecule::setup_particle(p);

  // Set provenance of this chain
  core::StructureProvenance sp
          = core::StructureProvenance::setup_particle(new Particle(m),
                                                      filename, chain_id);
  core::add_provenance(m, p->get_index(), sp);

  return p;
}

bool is_ATOM_rec(const String& pdb_line) {
  return (pdb_line[0] == 'A' && pdb_line[1] == 'T' && pdb_line[2] == 'O' &&
          pdb_line[3] == 'M');
}

bool is_HETATM_rec(const String& pdb_line) {
  return (pdb_line[0] == 'H' && pdb_line[1] == 'E' && pdb_line[2] == 'T' &&
          pdb_line[3] == 'A' && pdb_line[4] == 'T' && pdb_line[5] == 'M');
}

bool is_MODEL_rec(const String& pdb_line) {
  return (pdb_line[0] == 'M' && pdb_line[1] == 'O' && pdb_line[2] == 'D' &&
          pdb_line[3] == 'E' && pdb_line[4] == 'L');
}

bool is_ENDMDL_rec(const String& pdb_line) {
  return (pdb_line[0] == 'E' && pdb_line[1] == 'N' && pdb_line[2] == 'D' &&
          pdb_line[3] == 'M' && pdb_line[4] == 'D' && pdb_line[5] == 'L');
}

bool is_CONECT_rec(const String& pdb_line) {
  return (pdb_line[0] == 'C' && pdb_line[1] == 'O' && pdb_line[2] == 'N' &&
          pdb_line[3] == 'E' && pdb_line[4] == 'C' && pdb_line[5] == 'T');
}

int model_index(const String& pdb_line) {
  return atoi(pdb_line.substr(model_index_field_).c_str());
}

int atom_number(const String& pdb_line) {
  return atoi(pdb_line.substr(atom_number_field_).c_str());
}

String atom_type(const String& pdb_line) {
  String atom_type_string = pdb_line.substr(atom_type_field_, 4);
  return atom_type_string;
}

char atom_alt_loc_indicator(const String& pdb_line) {
  return pdb_line[atom_alt_loc_field_];
}

String atom_residue_name(const String& pdb_line) {
  String res_name = pdb_line.substr(atom_res_name_field_, 3);
  return res_name;
}

char atom_chain_id(const String& pdb_line) {
  return pdb_line[atom_chain_id_field_];
}

short atom_residue_number(const String& pdb_line) {
  return atoi(pdb_line.substr(atom_res_number_field_, 4).c_str());
}

char atom_residue_icode(const String& pdb_line) {
  return pdb_line[atom_res_insertion_field_];
}

float atom_xcoord(const String& pdb_line) {
  return atof(pdb_line.substr(atom_xcoord_field_, 8).c_str());
}

float atom_ycoord(const String& pdb_line) {
  return atof(pdb_line.substr(atom_ycoord_field_, 8).c_str());
}

float atom_zcoord(const String& pdb_line) {
  return atof(pdb_line.substr(atom_zcoord_field_, 8).c_str());
}

float atom_occupancy(const String& pdb_line) {
  if (pdb_line.length() >= atom_occupancy_field_ + 6)
    return atof(pdb_line.substr(atom_occupancy_field_, 6).c_str());
  return 0.0;
}

float atom_temp_factor(const String& pdb_line) {
  if (pdb_line.length() >= atom_temp_factor_field_ + 6)
    return atof(pdb_line.substr(atom_temp_factor_field_, 6).c_str());
  return 0.0;
}

String atom_element(const String& pdb_line) {
  String element_name;
  if (pdb_line.length() >= atom_element_field_ + 2)
    element_name = pdb_line.substr(atom_element_field_, 2);
  return element_name;
}

Vector<unsigned short> connected_atoms(const String& pdb_line) {
  Vector<unsigned short> conn_atoms;
  if (is_CONECT_rec(pdb_line)) {
    // (1-6) - "CONECT"
    // (7-11), (12-16), ...,(57-61) - serial numbers of connected atoms
    const unsigned short atom_index_len = 5;
    unsigned short atom_index_start = 6;
    unsigned short line_len = pdb_line.length();
    if (line_len > 61) line_len = 61;

    while ((atom_index_start + atom_index_len) <= line_len) {
      String atom_index_str = pdb_line.substr(atom_index_start, atom_index_len);
      short sh_atom_index = atoi(atom_index_str.c_str());
      if (sh_atom_index > 0) conn_atoms.push_back(sh_atom_index);
      atom_index_start += atom_index_len;
    }
  }
  return conn_atoms;
}

void write_pdb(const ParticlesTemp& ps, TextOutput out) {
  IMP_FUNCTION_LOG;
  int last_index = 0;
  bool use_input_index = true;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (Atom(ps[i]).get_input_index() != last_index + 1) {
      use_input_index = false;
      break;
    } else {
      ++last_index;
    }
  }
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (Atom::get_is_setup(ps[i])) {
      Atom ad(ps[i]);
      Residue rd = get_residue(ad);
      // really dumb and slow, fix later
      char chain;
      Chain c = get_chain(rd);
      if (c) {
        chain = c.get_id()[0];
      } else {
        chain = ' ';
      }
      int inum = i+1;
      if (i>=99999) inum=99999;
      out.get_stream() << get_pdb_string(
                              core::XYZ(ps[i]).get_coordinates(),
                              use_input_index ? ad.get_input_index()
                                              : static_cast<int>(inum),
                              ad.get_atom_type(), rd.get_residue_type(), chain,
                              rd.get_index(), rd.get_insertion_code(),
                              ad.get_occupancy(), ad.get_temperature_factor(),
                              ad.get_element());

      if (!out) {
        IMP_THROW("Error writing to file in write_pdb", IOException);
      }
    }
    else if (Residue::get_is_setup(ps[i])) {    // if C-alpha residue is available
      Residue rd = IMP::atom::Residue(ps[i]);
      // comment 1 by SJ - TODO: How to retrieve the correct chain information without an hierarchy?
      char chain;
      Chain c = get_chain(rd);
      if (c) {
        chain = c.get_id()[0];
      } else {
        chain = ' ';
      }

      // comment 2 by SJ - TODO: The C-alpha residues are not sorted yet. We need to sort the residues similarly to what PMI does.
      out.get_stream() << get_pdb_string(
                              core::XYZ(ps[i]).get_coordinates(),
                              static_cast<int>(i + 1),
                              IMP::atom::AT_CA, rd.get_residue_type(), chain,
                              rd.get_index(), ' ',
                              1.0, IMP::core::XYZR(ps[i]).get_radius());
    }
    else {  // if a coarse-grained BEAD is available
      Ints resindexes = IMP::atom::Fragment(ps[i]).get_residue_indexes();
      int resindex = (int)resindexes.front() + (int)(resindexes.size()/2);
      // comment 1 by SJ - TODO: How to retrieve the correct chain information without an hierarchy?
      char chain = ' ';

      // comment 3 by SJ - TODO: The BEADs are not sorted yet. We need to sort the residues similarly to what PMI does.
      // comment 4 by SJ - TODO: currently IMP does not allow "BEA" as a residue name, while PMI allows it. Thus "UNK" was used instead.
      out.get_stream() << get_pdb_string(
                              core::XYZ(ps[i]).get_coordinates(),
                              static_cast<int>(i + 1),
                              IMP::atom::AT_CA, IMP::atom::UNK, chain,
                              (int)resindex, ' ',
                              1.0, IMP::core::XYZR(ps[i]).get_radius());
    }
  }
}

IMPATOM_END_INTERNAL_NAMESPACE
