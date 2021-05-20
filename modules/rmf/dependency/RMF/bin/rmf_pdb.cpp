/**
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include <boost/format.hpp>  // IWYU pragma: keep
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/Vector.h"
#include "RMF/compiler_macros.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/sequence.h"
#include "common.h"

RMF_ENABLE_WARNINGS namespace {
  std::string description =
      "Convert an rmf file into an pdb file suitable for "
      "opening in a pdb viewer.";

  std::string element_names[] = {
      "H",  "HE", "LI", "BE", "B",  "C",  "N",  "O",  "F",  "NE", "NA", "MG",
      "AL", "SI", "P",  "S",  "CL", "AR", "K",  "CA", "SC", "TI", "V",  "CR",
      "MN", "FE", "CO", "NI", "CU", "ZN", "GA", "GE", "AS", "SE", "BR", "KR",
      "RB", "SR", "Y",  "ZR", "NB", "MO", "TC", "RU", "RH", "PD", "AG", "CD",
      "IN", "SN", "SB", "TE", "I",  "XE", "CS", "BA", "LA", "CE", "PR", "ND",
      "PM", "SM", "EU", "GD", "TB", "DY", "HO", "ER", "TM", "YB", "LU", "HF",
      "TA", "W",  "RE", "OS", "IR", "PT", "AU", "HG", "TL", "PB", "BI", "PO",
      "AT", "RN", "FR", "RA", "AC", "TH", "PA", "U",  "NP", "PU", "AM", "CM",
      "BK", "CF", "ES", "FM", "MD", "NO", "LR", "Db", "JL", "RF", };

  // change atom type to string for Hao's hetatom code
  std::string get_pdb_string(double x, double y, double z, int index,
                             std::string atom_name, std::string rt, char chain,
                             int res_index, char res_icode, double occupancy,
                             double tempFactor, std::string element_name) {
    std::stringstream out;

    if (atom_name.find("HET:") == 0) {
      out << "HETATM";
    } else {
      out << "ATOM  ";
    }
    // 7-11 : atom id
    out.setf(std::ios::right, std::ios::adjustfield);
    out.width(5);
    out << index;
    // 12: skip an undefined position
    out.width(1);
    out << " ";
    // 13-16: atom name
    if (atom_name.find("HET:") == 0) {
      atom_name.erase(0, 4);
    }
    if (atom_name.size() >= 4) {
      out << atom_name.substr(0, 4);
    } else if (element_name.size() == 2) {
      // left align atom names that have 2-character element names (e.g.
      // this distinguishes calcium CA from C-alpha, or mercury from H-gamma)
      out << std::left << std::setw(4) << atom_name;
    } else if (atom_name.size() == 3) {
      out << " " << atom_name;
    } else if (atom_name.size() == 2) {
      out << " " << atom_name << " ";
    } else {
      out << " " << atom_name << "  ";
    }
    // 17: skip the alternate indication position
    out.width(1);
    out << " ";
    // 18-20 : residue name
    out << std::right << std::setw(3) << rt.substr(0, 3);
    // skip 21
    out.width(1);
    out << " ";
    // 22: chain identifier
    out << chain;
    // 23-26: residue number
    out.setf(std::ios::right, std::ios::adjustfield);
    out.width(4);
    out << res_index;
    // 27: residue insertion code
    out.width(1);
    out << res_icode;
    out.setf(std::ios::fixed, std::ios::floatfield);
    out << "   ";  // skip 3 undefined positions (28-30)
                   // coordinates (31-38,39-46,47-54)
    out.width(8);
    out.precision(3);
    out << x;
    out.width(8);
    out.precision(3);
    out << y;
    out.width(8);
    out.precision(3);
    out << z;
    // 55:60 occupancy
    out.width(6);
    out.precision(2);
    out << occupancy;
    // 61-66: temp. factor
    out.width(6);
    out.precision(2);
    out << tempFactor;
    // 73 - 76  LString(4)      Segment identifier, left-justified.
    out.width(10);
    out << "";  // TODO
    // 77 - 78  LString(2)      Element symbol, right-justified.
    out.width(2);
    out.setf(std::ios::right, std::ios::adjustfield);
    out << element_name;
    //     79 - 80        LString(2)      Charge on the atom.
    out.width(2);
    out << "" << std::endl;  // TODO
    return out.str();
  }

  int write_atoms(
      std::ostream & out, int current_index, RMF::NodeConstHandle nh,
      RMF::decorator::IntermediateParticleFactory ipf,
      RMF::decorator::AtomFactory af, RMF::decorator::ChainFactory cf,
      RMF::decorator::ResidueFactory rf, std::string chain = std::string(),
      int residue_index = -1, std::string residue_type = std::string()) {
    if (cf.get_is(nh)) {
      chain = cf.get(nh).get_chain_id();
    }
    if (rf.get_is(nh)) {
      RMF::decorator::ResidueConst r = rf.get(nh);
      residue_index = r.get_residue_index();
      residue_type = r.get_residue_type();
    }
    if (af.get_is(nh)) {
      RMF::decorator::AtomConst a = af.get(nh);
      RMF::Vector3 coords = ipf.get(nh).get_coordinates();
      int element = a.get_element();
      // not safe
      std::string element_name = element_names[element - 1];
      std::string str = get_pdb_string(
          coords[0], coords[1], coords[2], ++current_index, nh.get_name(),
          residue_type, (chain == std::string() ? ' ' : (chain[0])),
          residue_index, ' ', 1.0, 0.0, element_name);
      out << str;
    }
    RMF::NodeConstHandles ch = nh.get_children();
    for (unsigned int i = 0; i < ch.size(); ++i) {
      current_index = write_atoms(out, current_index, ch[i], ipf, af, cf, rf,
                                  chain, residue_index, residue_type);
    }
    return current_index;
  }
}

int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("pdb");
    RMF_ADD_FRAMES;
    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    std::ostream* out;
    std::ofstream fout;
    if (!output.empty()) {
      fout.open(output.c_str());
      if (!fout) {
        std::cerr << "Error opening file " << output << std::endl;
        return 1;
      }
      out = &fout;
    } else {
      out = &std::cout;
    }
    RMF::decorator::IntermediateParticleFactory ipf(rh);
    RMF::decorator::AtomFactory af(rh);
    RMF::decorator::ChainFactory cf(rh);
    RMF::decorator::ResidueFactory rf(rh);
    RMF::NodeConstHandle rn = rh.get_root_node();
    for (unsigned int input_frame = start_frame, output_frame = 0;
         input_frame < rh.get_number_of_frames();
         input_frame += step_frame, ++output_frame) {
      rh.set_current_frame(RMF::FrameID(input_frame));
      *out << (boost::format("MODEL%1$9d") % (output_frame + 1)) << std::endl;
      write_atoms(*out, 0, rn, ipf, af, cf, rf);
      *out << "ENDMDL" << output_frame + 1 << std::endl;
    }
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
