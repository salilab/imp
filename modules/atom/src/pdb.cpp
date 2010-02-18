/**
 *  \file PDBParser.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/pdb.h>
#include <IMP/atom/internal/pdb.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/element.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <iomanip>

IMPATOM_BEGIN_NAMESPACE


PDBSelector::~PDBSelector(){}

namespace {

  // try putting the numbers after
  std::string try_rename(std::string str) {
    std::string out;
    for (unsigned int i=0; i< str.size(); ++i) {
      if (std::isalpha(str[i])) out.push_back(str[i]);
    }
    for (unsigned int i=0; i< str.size(); ++i) {
      if (!std::isalpha(str[i])) out.push_back(str[i]);
    }
    return out;
  }

Particle* atom_particle(Model *m, const std::string& pdb_line)
{
  AtomType atom_name;
  std::string string_name = internal::atom_type(pdb_line);
  boost::trim(string_name);
  if (pdb_line[0]!='A'){
    string_name= "HET_"+string_name;
    if (!atom_type_exists(string_name)) {
      std::string elem= internal::atom_element(pdb_line);
      boost::trim(elem);
      Element e= get_element_table().get_element(elem);
      if (e == UNKNOWN_ELEMENT) {
        IMP_LOG(VERBOSE,"Unable to parse element from line: "
                <<pdb_line << ": got \"" << elem << "\"\nSkipping.");
        return NULL;
      }
      atom_name=add_atom_type(string_name, e);
    } else {
      atom_name=AtomType(string_name);
    }
  } else {
    if (!AtomType::get_key_exists(string_name)) {
      string_name= try_rename(string_name);
      if (!AtomType::get_key_exists(string_name)) {
        IMP_LOG(VERBOSE, "ATOM record type not found: \"" << string_name
                << "\" from " << pdb_line << std::endl);
        return NULL;
      }
    }
    atom_name = AtomType(string_name);
  }
  Particle* p = new Particle(m);

  algebra::Vector3D v(internal::atom_xcoord(pdb_line),
                      internal::atom_ycoord(pdb_line),
                      internal::atom_zcoord(pdb_line));
  // atom decorator
  Atom d = Atom::setup_particle(p, atom_name);
  p->set_name(std::string("Atom "+ atom_name.get_string()));
  core::XYZ::setup_particle(p, v).set_coordinates_are_optimized(true);
  d.set_input_index(internal::atom_number(pdb_line));
  IMP_IF_CHECK(USAGE) {
    std::string name= internal::atom_element(pdb_line);
    boost::trim(name);
    if (!name.empty()) {
      Element e= get_element_table().get_element(name);
      if (e != UNKNOWN_ELEMENT) {
        if (e != d.get_element()) {
          IMP_WARN("Read and computed elements don't match. Read " << e
                   << " Computed " << d.get_element()
                   << " from line " << pdb_line << std::endl);
        }
      }
    }
  }
  return p;
}

Particle* residue_particle(Model *m, const std::string& pdb_line)
{
  Particle* p = new Particle(m);

  int residue_index = internal::atom_residue_number(pdb_line);
  char residue_icode = internal::atom_residue_icode(pdb_line);
  std::string rn = internal::atom_residue_name(pdb_line);
  boost::trim(rn);
  ResidueType residue_name = ResidueType(rn);

  // residue decorator
  Residue rd =
    Residue::setup_particle(p, residue_name,
                             residue_index, (int)residue_icode);
  p->set_name(residue_name.get_string());
  return p;
}

Particle* chain_particle(Model *m, char chain_id)
{
  Particle* p = new Particle(m);
  Chain::setup_particle(p, chain_id);
  p->set_name(std::string("Chain "+std::string(1, chain_id)));
  return p;
}

void set_chain_name(const Hierarchy& hrd, Hierarchy& hcd)
{
}

}


namespace {

Hierarchies read_pdb(std::istream &in, Model *model,
                   const PDBSelector& selector,
                   bool select_first_model,
                   bool split_models)
{
  // hierarchy decorator
  Hierarchies ret;
  std::string root_name;
  Particle* root_p = NULL;
  Particle* cp = NULL;
  Particle* rp = NULL;

  char curr_residue_icode = '-';
  char curr_chain = '-';
  bool chain_name_set = false;
  bool first_model_read = false;
  bool has_atom=false;

  std::string line;
  while (!in.eof()) {
    if (!getline(in, line)) break;

    // handle MODEL reading
    if (internal::is_MODEL_rec(line)) {
      if(first_model_read && select_first_model) break;
      if (split_models) {
        std::ostringstream oss;
        oss << "Model " << internal::model_index(line);
        root_name= oss.str();
        root_p=NULL;
      }
    }

    // check that line is an HETATM or ATOM rec and that selector accepts line.
    // if this is the case construct a new Particle using line and add the
    // Particle to the Model
    if (internal::is_ATOM_rec(line) || internal::is_HETATM_rec(line)) {
      if (!selector(line)) {
        IMP_LOG(VERBOSE, "Selector rejected line " << line << std::endl);
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
        if (root_p== NULL) {
          root_p = new Particle(model);
          ret.push_back(Hierarchy::setup_particle(root_p));
          if (!root_name.empty()) {
            root_p->set_name(root_name);
          }
        }

        if (cp == NULL || chain != curr_chain) {
          curr_chain = chain;
          // create new chain particle
          cp = chain_particle(model, chain);
          chain_name_set = false;
          Hierarchy(root_p).add_child(Chain(cp));
          rp=NULL; // make sure we get a new residue
        }

        // check if new residue
        if (rp == NULL ||
            residue_index != Residue::decorate_particle(rp).get_index() ||
            residue_icode != curr_residue_icode) {
          curr_residue_icode = residue_icode;
          // create new residue particle
          rp = residue_particle(model, line);
          Chain(cp).add_child(Residue(rp));
        }

        // set chain name (protein/nucleotide/other) according to residue name
        if (!chain_name_set) {
          Chain cd(cp);
          set_chain_name(Residue(rp), cd);
          chain_name_set = true;
        }

        Residue(rp).add_child(Atom(ap));
        has_atom=true;
      }

    }
  }
  if (!has_atom) {
    throw IOException("Sorry, unable to read atoms from PDB file."
                      " Thanks for the effort.");
  }
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< ret.size(); ++i) {
      if (!ret[i].get_is_valid(true)) {
        IMP_ERROR("Invalid hierarchy produced ");
        IMP_ERROR_WRITE(IMP::core::show<Hierarchy>(ret[i], IMP_STREAM));
        throw InternalException("Bad hierarchy");
        // should clean up
      }
    }
  }
  return ret;
}
}

Hierarchy read_pdb(TextInput in, Model *model) {
  return read_pdb(in, model, NonWaterPDBSelector(), true, false)[0];
}



Hierarchy read_pdb(TextInput in, Model *model,
                   const PDBSelector& selector,
                   bool select_first_model)
{
  return read_pdb(in, model, selector, select_first_model, false)[0];
}



Hierarchies read_multimodel_pdb(TextInput in, Model *model,
                   const PDBSelector& selector)
{
  return read_pdb(in, model, selector, false, true);
}

// mol2.cpp
bool check_arbond(Particle* atom_p);

void write_pdb(const Particles& ps, TextOutput out)
{
  int last_index=0;
  bool use_input_index=true;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (Atom(ps[i]).get_input_index() != last_index+1) {
      use_input_index=false;
      break;
    } else {
      ++last_index;
    }
  }
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (Atom::particle_is_instance(ps[i])) {
      Atom ad(ps[i]);
      Residue rd= get_residue(ad);
      // really dumb and slow, fix later
      char chain;
      try {
        Chain c=get_chain(rd);
        chain= c.get_id();
      } catch (const ValueException &) {
        /*std::string atom_type = get_atom_type().get_string();
        if (atom_type[0] == 'O' || atom_type[0] == 'S') {
          if (check_arbond(p)) {
            String stratype (1, atom_type[0]);
            atom_type = stratype + ".ar";
          }
        }
        if (atom_type.find('.') != String::npos) {
          atom_type.erase(atom_type.find('.'), 1);
        }
        out.get_stream() << pdb_string(core::XYZ(ps[i]).get_coordinates(),
                                       use_input_index? ad.get_input_index(): i,
                                       atom_type,
                                       rd.get_residue_type(),
                                       c.get_id(),
                                       rd.get_index(),
                                       rd.get_insertion_code(),
                                       ad.get_element());*/
        chain=' ';
        /*
        pdb_file << std::setw(7) << atomid << " ";
        pdb_file.setf(std::ios::left,std::ios::adjustfield);
        pdb_file << std::setw(4) << atom_type << " ";
        pdb_file << "HET     1" << "    ";
        pdb_file.setf(std::ios::right,std::ios::adjustfield);
        pdb_file.setf(std::ios::fixed,std::ios::floatfield);
        core::XYZ xyz= core::XYZ::cast(p);
        pdb_file << std::setw(8) << std::setprecision(3) << xyz.get_x();
        pdb_file << std::setw(8) << std::setprecision(3) << xyz.get_y();
        pdb_file << std::setw(8) << std::setprecision(3) << xyz.get_z()
        << std::endl;*/
      }
      out.get_stream() << pdb_string(core::XYZ(ps[i]).get_coordinates(),
                                     use_input_index? ad.get_input_index(): i,
                                     ad.get_atom_type().get_string(),
                                     rd.get_residue_type(),
                                     chain,
                                     rd.get_index(),
                                     rd.get_insertion_code(),
                                     ad.get_element());

      if (!out) {
        IMP_THROW("Error writing to file in write_pdb",
                  IOException);
      }
    }
  }
}

void write_pdb(Hierarchy mhd, TextOutput out)
{
  Particles ps= get_leaves(mhd);
  write_pdb(ps, out);
}

void write_pdb(const Hierarchies& mhd, TextOutput out)
{
  for (unsigned int i=0; i< mhd.size(); ++i) {
    write_pdb(mhd[i], out);
  }
}


void write_multimodel_pdb(const Hierarchies& mhd, TextOutput oout)
{
  std::ostream &out=oout;
  for (unsigned int i=0; i< mhd.size(); ++i) {
    out << "MODEL   ";
    out.setf(std::ios::right, std::ios::adjustfield);
    out.width(4);
    out << (i+1) << std::endl;
    write_pdb(mhd[i], out);
    out << "ENDMDL" << std::endl;
  }
}



// change atom type to string for Hao's hetatom code
std::string pdb_string(const algebra::Vector3D& v, int index,
                       AtomType at, ResidueType rt,
                       char chain, int res_index,
                       char res_icode, Element e) {
  std::stringstream out;
  if (rt > DTHY) {
    out << "HETATM";
  } else {
    out << "ATOM  ";
  }
  //7-11 : atom id
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(5);
  out << index;
  // 12: skip an undefined position
  out.width(1);
  out << " ";
  // 13-16: atom name
  out.setf(std::ios::left, std::ios::adjustfield);
  out.width(1);
  std::string atom_name = at.get_string();
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(4);
  if (atom_name.size()<4) {
    out << atom_name;
  } else if (atom_name.find("HET_")==0){
    out << std::string(atom_name,4);
  } else {
    out << atom_name;
  }
  // 17: skip the alternate indication position
  out.width(1);
  out << " ";
  // 18-20 : residue name
  out << std::right << std::setw(3) << rt.get_string();
  //skip 21
  out.width(1);
  out << " ";
  // 22: chain identifier
  out << chain;
  //23-26: residue number
  out.setf(std::ios::right, std::ios::adjustfield);
  out.width(4);
  out << res_index;
  //27: residue insertion code
  out.width(1);
  out << res_icode;
  out.setf(std::ios::fixed, std::ios::floatfield);
  out << "   "; // skip 3 undefined positions (28-30)
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
  //55:60 occupancy
  out.width(6);
  out.precision(2);
  out << ""; //TODO
  //61-66: temp. factor
  out.width(6);
  out.precision(2);
  out << ""; //TODO
  // 73 - 76  LString(4)      Segment identifier, left-justified.
  out.width(10);
  out << ""; //TODO
  // 77 - 78  LString(2)      Element symbol, right-justified.
  out.width(2);
  out.setf(std::ios::right, std::ios::adjustfield);
  out << get_element_table().get_name(e);
  //     79 - 80        LString(2)      Charge on the atom.
  out.width(2);
  out << "" << std::endl; //TODO
  return out.str();
}

std::string conect_record_string(int a1_ind,int a2_ind){
  //      const IMP::atom::Atom &a1, const IMP::atom::Atom &a2){
  //  IMP::atom::Atom *a3,IMP::atom::Atom *a4,IMP::atom::Atom *a5) {

  std::stringstream out;
  out.setf(std::ios::left, std::ios::adjustfield);
  // 1-6         Record name      "CONECT"
  out.width(6);
  out<<"CONECT";
  //7 - 11 Atom serial number
  out.width(5);
  out<<a1_ind;//a1.get_input_index();
  //12 - 16 Serial number of bonded atom
  out<<a2_ind;//a2.get_input_index();
  //17 - 21 Serial number of bonded atom
  // if(a3 != NULL) {
  //   out<<a3->get_input_index();
  // }
  // //22 - 26  Serial number of bonded atom
  // if(a4 != NULL) {
  //   out<<a4->get_input_index();
  // }
  // //27 - 31 Serial number of bonded atom
  // if(a5 != NULL) {
  //   out<<a5->get_input_index();
  // }
  return out.str();
}

IMPATOM_END_NAMESPACE
