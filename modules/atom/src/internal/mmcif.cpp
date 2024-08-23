/**
 *  \file internal/mmcif.cpp
 *  \brief Utility functions for handling mmCIF files.
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/internal/mmcif.h>
#include <fstream>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

namespace {
  std::string cifquote(const std::string &s) {
    // We may need to quote the string, e.g. if it contains quotes or
    // is a CIF keyword
    if (s.length() > 0 && s.find_first_of("'\" ") == std::string::npos
        && s[0] != '_' && s.find("global_") != 0
        && s[0] != '[' && s.find("data_") != 0
        && s.find("save_") != 0 && s != "loop_"
        && s != "stop_" && s != "?" && s != ".") {
      return s;
    } else if (s.find('"') != std::string::npos) {
      return "'" + s + "'";
    } else {
      return "\"" + s + "\"";
    }
  }
}

AtomSiteDumper::AtomSiteDumper(std::ofstream &out_file)
    : out_file_(out_file), header_written_(false)
{
}

void AtomSiteDumper::write_header()
{
  out_file_ << "loop_" << std::endl
    << "_atom_site.group_PDB" << std::endl
    << "_atom_site.type_symbol" << std::endl
    << "_atom_site.label_atom_id" << std::endl
    << "_atom_site.label_alt_id" << std::endl
    << "_atom_site.label_comp_id" << std::endl
    << "_atom_site.label_asym_id" << std::endl
    << "_atom_site.auth_asym_id" << std::endl
    << "_atom_site.label_seq_id" << std::endl
    << "_atom_site.auth_seq_id" << std::endl
    << "_atom_site.pdbx_PDB_ins_code" << std::endl
    << "_atom_site.Cartn_x" << std::endl
    << "_atom_site.Cartn_y" << std::endl
    << "_atom_site.Cartn_z" << std::endl
    << "_atom_site.occupancy" << std::endl
    << "_atom_site.B_iso_or_equiv" << std::endl
    << "_atom_site.label_entity_id" << std::endl
    << "_atom_site.id" << std::endl
    << "_atom_site.pdbx_PDB_model_num" << std::endl;
}

void AtomSiteDumper::write(const algebra::Vector3D &v, const Atom &a,
                           int model_num)
{
  if (!header_written_) {
    write_header();
    header_written_ = true;
  }

  Residue r = get_residue(a);
  ResidueType rt = r.get_residue_type();
  Chain c = get_chain(r);

  std::string atom_name = a.get_atom_type().get_string();
  std::string element_name = get_element_table().get_name(a.get_element());

  if (atom_name.find("HET:") == 0) {
    out_file_ << "HETATM ";
    atom_name.erase(0, 4);
  } else {
    out_file_ << "ATOM ";
  }
  int res_index = r.get_index();
  char inscode = r.get_insertion_code();
  std::string asym_id = c.get_label_asym_id();
  out_file_.precision(3);
  out_file_ << cifquote(element_name) << " " << cifquote(atom_name)
      // skip the alternate location, as we ignore these in IMP
      << " . "
      << cifquote(rt.get_string()) << " "
      << cifquote(asym_id.length() > 0 ? asym_id : c.get_id())
      << " " << cifquote(c.get_id()) << " "
      << (res_index - c.get_sequence_offset()) << " "
      << res_index << " " << (inscode == ' ' ? '?' : inscode) << " "
      << std::fixed
      << v[0] << " " << v[1] << " " << v[2] << " " << a.get_occupancy()
      << " " << a.get_temperature_factor()
      // We don't have an entity ID, so just report 1 for everything for now
      // (or we could assign each unique chain a different integer)
      << " 1 "
      << a.get_input_index() << " " << model_num << std::endl;
}

IMPATOM_END_INTERNAL_NAMESPACE
