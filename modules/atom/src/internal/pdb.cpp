/**
 *  \file PDBParser.h
 *  \brief A class with static functions for parsing PDB files
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/internal/pdb.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/pdb.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

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

base::Vector<unsigned short> connected_atoms(const String& pdb_line) {
  base::Vector<unsigned short> conn_atoms;
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

void write_pdb(const kernel::ParticlesTemp& ps, base::TextOutput out) {
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
      out.get_stream() << get_pdb_string(
                              core::XYZ(ps[i]).get_coordinates(),
                              use_input_index ? ad.get_input_index()
                                              : static_cast<int>(i + 1),
                              ad.get_atom_type(), rd.get_residue_type(), chain,
                              rd.get_index(), rd.get_insertion_code(),
                              ad.get_occupancy(), ad.get_temperature_factor(),
                              ad.get_element());

      if (!out) {
        IMP_THROW("Error writing to file in write_pdb", IOException);
      }
    }
  }
}

IMPATOM_END_INTERNAL_NAMESPACE
