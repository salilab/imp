/**
 *  \file PDBParser.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/pdb.h>
#include <IMP/atom/internal/pdb.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/element.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/charmm_segment_topology.h>
#include <IMP/core/provenance.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/core/rigid_bodies.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <locale>
#include <fstream>

#include <iomanip>

#include <boost/version.hpp>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

IMPATOM_BEGIN_NAMESPACE

bool HydrogenPDBSelector::is_hydrogen(std::string pdb_line) const {
  if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
    return false;
  }
  std::string elem = internal::atom_element(pdb_line);
  boost::trim(elem);
  // determine if the line is hydrogen atom as follows:
  // 1. if the record has element field (columns 76-77),
  // check that it is indeed H. Note that it may be missing
  // in some files.
  // some programs do not output element, so the ATOM
  // line can be shorter.
  if (elem.length() == 1 && elem[0] == 'H') return true;
  // 2. support elements that starts with H: He, Ho, Hf, Hg
  if (elem.length() == 2 && elem[0] == 'H' &&
      (elem[1] == 'E' || elem[1] == 'e' || elem[1] == 'O' || elem[1] == 'o' ||
       elem[1] == 'F' || elem[1] == 'f' || elem[1] == 'G' || elem[1] == 'g'))
    return false;
  // 3. if no hydrogen is found in the element record,
  // try atom type field.
  // some NMR structures have 'D' for labeled hydrogens
  std::string atom_name = internal::atom_type(pdb_line);
  return (  // " HXX" or " DXX" or "1HXX" ...
      ((atom_name[0] == ' ' || isdigit(atom_name[0])) &&
       (atom_name[1] == 'H' || atom_name[1] == 'D')) ||
      // "HXXX" or "DXXX"
      (atom_name[0] == 'H' || atom_name[0] == 'D'));
}

namespace {
std::string nicename(std::string name) {
  boost::filesystem::path path(name);
  return path.string();
}
}

PDBSelector::~PDBSelector() {}

namespace {

Element get_element_from_pdb_line(const std::string& pdb_line) {
  // 1. determine element from element column
  std::string elem = internal::atom_element(pdb_line);
  boost::trim(elem);
  Element e = get_element_table().get_element(elem);
  if (e != UNKNOWN_ELEMENT) return e;

  // 2. determine element from the atom name
  std::string atom_name = internal::atom_type(pdb_line);
  IMP_USAGE_CHECK(atom_name.length() == 4, "Invalid atom name.");

  if (internal::is_ATOM_rec(pdb_line)) {
    char c0 = atom_name[0];
    // if we have space/digit followed by character, try character
    if ((isdigit(atom_name[0]) || isspace(atom_name[0])) &&
        isalpha(atom_name[1]))
      c0 = atom_name[1];
    // H, C, N, O, S, P - most likely to occur
    // we don't want to get Ne element for NE AtomType
    switch (c0) {
      case 'H':
        return H;
      case 'C':
        return C;
      case 'N':
        return N;
      case 'O':
        return O;
      case 'S':
        return S;
      case 'P':
        return P;
      default:
        break;
    }
  }

  if (internal::is_HETATM_rec(pdb_line)) {
    // if we have a character in position 0 try the first two characters
    // in order to handle cases below correctly
    // HETATM 2209 MG    MG D 392      12.055 -14.213 -59.777  1.00 10.93
    // HETATM    1 CL    CL E 393     -24.319  -2.178 -56.585  0.40  2.37
    // HETATM    2  O   HOH E   1     -25.636  -4.048 -37.445  1.00  4.32
    std::string elem_string;
    if (isalpha(atom_name[0])) elem_string.push_back(atom_name[0]);
    if (isalpha(atom_name[1])) elem_string.push_back(atom_name[1]);
    e = get_element_table().get_element(elem_string);
    if (e != UNKNOWN_ELEMENT) return e;
  }

  IMP_LOG_VERBOSE("Unable to parse element from line: " << pdb_line << "\n");
  return UNKNOWN_ELEMENT;
}

Particle* atom_particle(Model* m, const std::string& pdb_line) {
  // determine element
  Element e = get_element_from_pdb_line(pdb_line);
  return internal::atom_particle(
                     m, internal::atom_type(pdb_line), e,
                     internal::is_HETATM_rec(pdb_line),
                     internal::atom_number(pdb_line),
                     internal::atom_residue_number(pdb_line),
                     internal::atom_xcoord(pdb_line),
                     internal::atom_ycoord(pdb_line),
                     internal::atom_zcoord(pdb_line),
                     internal::atom_occupancy(pdb_line),
                     internal::atom_temp_factor(pdb_line));
}

Particle* residue_particle(Model* m, const std::string& pdb_line) {
  return internal::residue_particle(m, internal::atom_residue_number(pdb_line),
                   internal::atom_residue_icode(pdb_line),
                   internal::atom_residue_name(pdb_line));
}

}

namespace {

Hierarchies read_pdb(std::istream& in, std::string name, std::string filename,
                     Model* model, PDBSelector* selector,
                     bool select_first_model, bool split_models, bool noradii) {
  IMP_FUNCTION_LOG;
  IMP::PointerMember<PDBSelector> sp(selector);
  // hierarchy decorator
  Hierarchies ret;
  std::string root_name;
  Particle* root_p = nullptr;
  Particle* cp = nullptr;
  Particle* rp = nullptr;

  char curr_residue_icode = '-';
  char curr_chain = '-';
  bool chain_name_set = false;
  bool first_model_read = false;
  bool has_atom = false;

  std::string line;
  while (!in.eof()) {
    getline(in, line);
    if (in.eof()) break;
    if (in.bad() || in.fail()) {
      IMP_THROW("Error reading from PDB file", IOException);
    }
    // handle MODEL reading
    if (internal::is_MODEL_rec(line)) {
      if (first_model_read && select_first_model) break;
      if (split_models) {
        std::ostringstream oss;
        oss << internal::model_index(line);
        root_name = oss.str();
        root_p = nullptr;
        cp = nullptr;
        rp = nullptr;
      }
      first_model_read = true;
    }

    // check that line is an HETATM or ATOM rec and that selector accepts line.
    // if this is the case construct a new Particle using line and add
    // the
    // Particle to the Model
    if (internal::is_ATOM_rec(line) || internal::is_HETATM_rec(line)) {
      if (!selector->get_is_selected(line)) {
        IMP_LOG_VERBOSE("Selector rejected line " << line << std::endl);
        continue;
      }
      int residue_index = internal::atom_residue_number(line);
      char residue_icode = internal::atom_residue_icode(line);
      char chain = internal::atom_chain_id(line);

      // create atom particle
      Particle* ap = atom_particle(model, line);
      // make sure that all children have coordinates,
      // (no residues without valid atoms)
      if (ap) {
        // check if new chain
        if (root_p == nullptr) {
          root_p = new Particle(model);
          ret.push_back(Hierarchy::setup_particle(root_p));
          if (!root_name.empty() || !name.empty()) {
            root_p->set_name(name + ": " + root_name);
          }
        }

        if (cp == nullptr || chain != curr_chain) {
          curr_chain = chain;
          // create new chain particle
          cp = internal::chain_particle(model, std::string(1, chain), filename);
          chain_name_set = false;
          Hierarchy(root_p).add_child(Chain(cp));
          rp = nullptr;  // make sure we get a new residue
        }

        // check if new residue
        if (rp == nullptr ||
            residue_index != Residue(rp).get_index() ||
            residue_icode != curr_residue_icode) {
          curr_residue_icode = residue_icode;
          // create new residue particle
          rp = residue_particle(model, line);
          Chain(cp).add_child(Residue(rp));
        }

        // set chain name (protein/nucleotide/other) according to residue name
        if (!chain_name_set) {
          Chain cd(cp);
          chain_name_set = true;
        }

        Residue(rp).add_child(Atom(ap));
        has_atom = true;
      }
    }
  }
  if (!has_atom) {
    IMP_WARN(
        "Sorry, unable to read atoms from PDB file."
        " Thanks for the effort.\n");
    return Hierarchies();
  }
  if (!noradii) {
    internal::add_pdb_radii(ret);
  }
  return ret;
}
}

void read_pdb(TextInput in, int model, Hierarchy h) {
  boost::unordered_map<int, Particle*> atoms_map;
  atom::Hierarchies atoms = get_by_type(h, ATOM_TYPE);
  boost::unordered_map<core::RigidBody, ParticleIndexes> rigid_bodies;
  for (unsigned int i = 0; i < atoms.size(); ++i) {
    atoms_map[atoms[i]->get_value(internal::get_pdb_index_key())] = atoms[i];
    if (core::RigidMember::get_is_setup(atoms[i])) {
      rigid_bodies[core::RigidMember(atoms[i]).get_rigid_body()]
          .push_back(atoms[i]->get_index());
    }
  }
  std::string line;
  bool reading = (model == 0);
  while (!in.get_stream().eof()) {
    getline(in.get_stream(), line);
    if (in.get_stream().eof()) break;
    if (in.get_stream().bad() || in.get_stream().fail()) {
      IMP_THROW("Error reading from PDB file", IOException);
    }
    // handle MODEL reading
    if (internal::is_MODEL_rec(line)) {
      int index = internal::model_index(line);
      if (index == model) {
        reading = true;
      } else {
        break;
      }
    }
    if (reading &&
        (internal::is_ATOM_rec(line) || internal::is_HETATM_rec(line))) {
      int index = internal::atom_number(line);
      if (atoms_map.find(index) != atoms_map.end()) {
        algebra::Vector3D v(internal::atom_xcoord(line),
                            internal::atom_ycoord(line),
                            internal::atom_zcoord(line));
        core::XYZ(atoms_map.find(index)->second).set_coordinates(v);
      }
    }
  }
  for (boost::unordered_map<core::RigidBody, ParticleIndexes>::iterator
           it = rigid_bodies.begin();
       it != rigid_bodies.end(); ++it) {
    core::RigidBody rb = it->first;
    rb.set_reference_frame_from_members(it->second);
  }
  if (!reading) {
    IMP_THROW("No model " << model << " found in file", ValueException);
  }
}

Hierarchy read_pdb(TextInput in, Model* model,
                   PDBSelector* selector, bool select_first_model,
                   bool no_radii) {
  IMP::PointerMember<PDBSelector> sp(selector);
  Hierarchies ret = read_pdb(in, nicename(in.get_name()), in.get_name(),
                             model, selector, select_first_model, false,
                             no_radii);
  if (ret.empty()) {
    IMP_THROW("No molecule read from file " << in.get_name(), ValueException);
  }
  return ret[0];
}

Hierarchies read_multimodel_pdb(TextInput in, Model* model,
                                PDBSelector* selector, bool noradii) {
  IMP::PointerMember<PDBSelector> sp(selector);
  Hierarchies ret = read_pdb(in, nicename(in.get_name()), in.get_name(),
                             model, selector, false, true, noradii);
  if (ret.empty()) {
    IMP_THROW("No molecule read from file " << in.get_name(), ValueException);
  }
  return ret;
}

void write_pdb(const Selection& mhd, TextOutput out, unsigned int model) {
  out.get_stream() << boost::format("MODEL%1$9d") % model << std::endl;
  internal::write_pdb(mhd.get_selected_particles(), out);
  out.get_stream() << "ENDMDL" << std::endl;
}

void write_multimodel_pdb(const Hierarchies& mhd, TextOutput out) {
  for (unsigned int i = 0; i < mhd.size(); ++i) {
    out.get_stream() << boost::format("MODEL%1$9d") % (i + 1) << std::endl;
    internal::write_pdb(get_leaves(mhd[i]), out);
    out.get_stream() << "ENDMDL" << std::endl;
  }
}

void write_pdb_of_c_alphas(const Selection& mhd, TextOutput out,
                           unsigned int model) {
  IMP_FUNCTION_LOG;
  out.get_stream() << boost::format("MODEL%1$9d") % model << std::endl;
  atom::Hierarchies leaves = get_leaves(mhd);
  int cur_residue = 0;
  for (unsigned int i = 0; i < leaves.size(); ++i) {
    ResidueType rt = ALA;
    if (Residue::get_is_setup(leaves[i])) {
      cur_residue = Residue(leaves[i]).get_index();
      rt = Residue(leaves[i]).get_residue_type();
    } else {
      cur_residue = cur_residue + 1;
    }
    char chain;
    Chain c = get_chain(leaves[i]);
    if (c) {
      chain = c.get_id()[0];
    } else {
      chain = ' ';
    }
    out.get_stream() << get_pdb_string(core::XYZ(leaves[i]).get_coordinates(),
                                       i + 1, AT_CA, rt, chain, cur_residue,
                                       ' ', 0.0, 0.0, C);

    if (!out) {
      IMP_THROW("Error writing to file in write_pdb", IOException);
    }
  }
  out.get_stream() << "ENDMDL" << std::endl;
}

// change atom type to string for Hao's hetatom code
std::string get_pdb_string(const algebra::Vector3D& v, int index, AtomType at,
                           ResidueType rt, char chain, int res_index,
                           char res_icode, double occupancy, double tempFactor,
                           Element e) {
  std::stringstream out;
  std::string atom_name = at.get_string();
  std::string element_name = get_element_table().get_name(e);

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
  } else if (e != UNKNOWN_ELEMENT && element_name.size() == 2) {
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
  out << std::right << std::setw(3) << rt.get_string().substr(0, 3);
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
  out << v[0];
  out.width(8);
  out.precision(3);
  out << v[1];
  out.width(8);
  out.precision(3);
  out << v[2];
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

std::string get_pdb_conect_record_string(int a1_ind, int a2_ind) {
  std::stringstream out;
  out.setf(std::ios::left, std::ios::adjustfield);
  // up to five atoms can be connected by CONECT line, we only support two
  // 1-6         Record name      "CONECT"
  out.width(6);
  out << "CONECT";
  // 7 - 11 Atom serial number
  out.width(5);
  out.setf(std::ios::right, std::ios::adjustfield);
  out << a1_ind;  // a1.get_input_index();
  // 12 - 16 Serial number of bonded atom
  out.width(5);
  out.setf(std::ios::right, std::ios::adjustfield);
  out << a2_ind;  // a2.get_input_index();
  out << std::endl;
  return out.str();
}

WritePDBOptimizerState::WritePDBOptimizerState(
    Model* m, const ParticleIndexesAdaptor& pis,
    std::string filename)
    : OptimizerState(m, filename + "Writer"),
      filename_(filename),
      pis_(pis) {}
WritePDBOptimizerState::WritePDBOptimizerState(const atom::Hierarchies mh,
                                               std::string filename)
    : OptimizerState(mh[0].get_model(), filename + "Writer"),
      filename_(filename) {
  IMP_FOREACH(atom::Hierarchy h, mh) { pis_.push_back(h.get_particle_index()); }
}

void WritePDBOptimizerState::do_update(unsigned int call) {
  std::ostringstream oss;
  bool append = (call != 0);
  std::string filename;
  Hierarchies hs;
  IMP_FOREACH(ParticleIndex pi, pis_) {
    hs.push_back(Hierarchy(get_model(), pi));
  }
  try {
    oss << boost::format(filename_) % call;
    append = false;
    filename = oss.str();
  }
  catch (...) {
  }
  if (append) {
    write_pdb(hs, TextOutput(filename, true), call);
  } else {
    write_pdb(hs, TextOutput(filename, false), 0);
  }
}

ModelObjectsTemp WritePDBOptimizerState::do_get_inputs() const {
  ModelObjectsTemp ret;
  IMP_FOREACH(ParticleIndex pi, pis_) {
    ret += get_leaves(Hierarchy(get_model(), pi));
  }
  return ret;
}

IMPATOM_END_NAMESPACE
