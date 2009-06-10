/**
 *  \file PDBParser.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/pdb.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Charmm.h>
#include <IMP/atom/internal/Topology.h>
#include <IMP/directories.h>

#include <boost/algorithm/string.hpp>

#include <fstream>

IMPATOM_BEGIN_NAMESPACE


Selector::~Selector(){}

namespace {

Particle* atom_particle(Model *m, const String& pdb_line)
{
  Particle* p = new Particle(m);

  algebra::Vector3D v(internal::atom_xcoord(pdb_line),
                      internal::atom_ycoord(pdb_line),
                      internal::atom_zcoord(pdb_line));

  String atom_name = internal::atom_type(pdb_line);
  boost::trim(atom_name);
  AtomType atom_type = AtomType(atom_name.c_str());

  // atom decorator
  Atom d = Atom::create(p, atom_type);
  core::XYZ::create(p, v).set_coordinates_are_optimized(true);
  d.set_input_index(internal::atom_number(pdb_line));

  return p;
}

Particle* residue_particle(Model *m, const String& pdb_line)
{
  Particle* p = new Particle(m);

  int residue_index = internal::atom_residue_number(pdb_line);
  char residue_icode = internal::atom_residue_icode(pdb_line);
  String residue_name = internal::atom_residue_name(pdb_line);
  ResidueType residue_type = ResidueType(residue_name.c_str());

  // residue decorator
  Residue rd =
    Residue::create(p, residue_type,
                             residue_index, (int)residue_icode);

  p->set_name(residue_name);

  return p;
}

Particle* root_particle(Model *m)
{
  Particle* p = new Particle(m);

  // hierarchy decorator
  Hierarchy hd =
    Hierarchy::create(p,
               Hierarchy::PROTEIN);
  return p;
}

Particle* chain_particle(Model *m, char chain_id)
{
  Particle* p = new Particle(m);
  Chain::create(p, chain_id);

  return p;
}

void set_chain_type(const Hierarchy& hrd,
                               Hierarchy& hcd) {

  if (hrd.get_type() == Hierarchy::RESIDUE)
    hcd.set_type(Hierarchy::CHAIN);
  else if (hrd.get_type() == Hierarchy::NUCLEICACID)
    hcd.set_type(Hierarchy::NUCLEOTIDE);
  else
    hcd.set_type(Hierarchy::MOLECULE);
}

}

Hierarchy read_pdb(
                             String pdb_file_name, Model *model,
                             const Selector& selector,
                             bool select_first_model,
                             bool ignore_alternatives)
{
  std::ifstream pdb_file(pdb_file_name.c_str());
  if (!pdb_file) {
    IMP_failure("No such PDB file " << pdb_file_name,
                ValueException);
  }
  Hierarchy root_d
      = read_pdb(pdb_file, model, selector, select_first_model,
                 ignore_alternatives);
  root_d.get_particle()->set_name(pdb_file_name);
  pdb_file.close();
  return root_d;
}

Hierarchy read_pdb(std::istream &in, Model *model,
                                     const Selector& selector,
                                     bool select_first_model,
                                     bool ignore_alternatives)
{
  // create root particle
  Particle* root_p = root_particle(model);
  Hierarchy root_d =
    Hierarchy::cast(root_p);

  Particle* cp = NULL;
  Particle* rp = NULL;
  Hierarchy hcd, hrd;

  char curr_residue_icode = '-';
  char curr_chain = '-';
  bool chain_type_set = false;
  bool first_model_read = false;

  String line;
  while (!in.eof()) {
    getline(in, line);
    // check that line is an HETATM or ATOM rec and that selector accepts line.
    // if this is the case construct a new Particle using line and add the
    // Particle to the Model

    // handle MODEL reading
    if (internal::is_MODEL_rec(line)) {
      if(first_model_read && select_first_model) break;
      first_model_read = true; continue;
    }

    if ((internal::is_ATOM_rec(line) || internal::is_HETATM_rec(line))
        && selector(line)) {

      int residue_index = internal::atom_residue_number(line);
      char residue_icode = internal::atom_residue_icode(line);
      char chain = internal::atom_chain_id(line);

      // check if new chain
      if (cp == NULL || chain != curr_chain) {
        curr_chain = chain;
        // create new chain particle
        cp = chain_particle(model, chain);
        chain_type_set = false;
        hcd = Hierarchy::cast(cp);
        root_d.add_child(hcd);
      }

      // check if new residue
      if (rp == NULL ||
          residue_index != Residue::cast(rp).get_index() ||
          residue_icode != curr_residue_icode) {
        curr_residue_icode = residue_icode;
        // create new residue particle
        rp = residue_particle(model, line);
        hrd = Hierarchy::cast(rp);
        hcd.add_child(hrd);
      }

      // set chain type (protein/nucleotide/other) according to residue type
      if (!chain_type_set) {
        set_chain_type(hrd, hcd);
        chain_type_set = true;
      }

      // check if alternatives should be skipped
      IgnoreAlternativesSelector sel;
      if(ignore_alternatives && !sel(line)) continue;

      // create atom particle
      Particle* ap = atom_particle(model, line);
      Hierarchy had =
        Hierarchy::cast(ap);
        hrd.add_child(had);
    }
  }
  return root_d;
}

void add_bonds(Hierarchy d, std::string topology_file_name)
{
  // we want the file to be read only once
  std::string file_name = IMP::get_data_directory() +"/atom/top.lib";
  static internal::Topology topology(file_name);

  if(!topology_file_name.empty()) {
    internal::Topology user_topology(topology_file_name);
    user_topology.add_bonds(d);
  } else {
    topology.add_bonds(d);
  }
}

void add_radius(Hierarchy d, std::string par_file_name,
                std::string top_file_name)
{
  std::string def_top_file_name = IMP::get_data_directory() +"/atom/top.lib";
  std::string def_par_file_name = IMP::get_data_directory() +"/atom/par.lib";

  if(!par_file_name.empty()) def_par_file_name = par_file_name;
  if(!top_file_name.empty()) def_top_file_name = top_file_name;

  Charmm charmm(def_par_file_name, def_top_file_name);
  charmm.add_radius(d);
}

void write_pdb(const Particles& ps, std::ostream &out)
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (Atom::is_instance_of(ps[i])) {
      Atom ad(ps[i]);
      out << ad.get_pdb_string();
    }
  }
}

void write_pdb(const Particles& ps, std::string file_name)
{
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_failure("Can't open file " << file_name << " for writing",
                ValueException);
  }
  write_pdb(ps, out_file);
  out_file.close();
}

void write_pdb(Hierarchy mhd, std::string file_name)
{
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_failure("Can't open file " << file_name << " for writing",
                ValueException);
  }
  write_pdb(mhd, out_file);
  out_file.close();
}

void write_pdb(Hierarchy mhd, std::ostream &out)
{
  Particles ps= get_leaves(mhd);
  write_pdb(ps, out);
}

void write_pdb(const Hierarchys& mhd, std::ostream &out)
{
  for (unsigned int i=0; i< mhd.size(); ++i) {
    write_pdb(mhd[i], out);
  }
}

void write_pdb(const Hierarchys& mhd, std::string file_name)
{
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_failure("Can't open file " << file_name << " for writing",
                ValueException);
  }
  write_pdb(mhd, out_file);
  out_file.close();
}

IMPATOM_END_NAMESPACE
